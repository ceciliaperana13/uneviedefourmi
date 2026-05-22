#include "../include/Dijkstra.hpp"

#include <queue>
#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace std::chrono;

// ==============
//  Constructeur
// ==============

AlgorithmeDijkstra::AlgorithmeDijkstra(const Fourmiliere* fourmiliere)
    : _fourmiliere(fourmiliere)
{
    if (!fourmiliere)
        throw invalid_argument("AlgorithmeDijkstra : pointeur fourmiliere nul");
}

// ====================================
//  executer — point d'entrée public
// ====================================

ResultatDijkstra AlgorithmeDijkstra::executer() {
    // Premier Dijkstra pour les stats (distances, chemin initial)
    ResultatDijkstra res = _dijkstra();
    res.nbTours = 0;

    res.chemin = _reconstruireChemin(
        res.predecesseurs,
        _fourmiliere->getVestibule(),
        _fourmiliere->getDortoir()
    );

    if (!res.chemin.empty())
        res.nbTours = _deplacerFourmis(res);

    return res;
}

// ============================================================
//  _calculerPoids
//  -1  → salle inaccessible (pleine)
//   1  → tunnel, dortoir, ou salle illimitée
//  cap / place_restante → salle avec capacité réelle
// ============================================================

int AlgorithmeDijkstra::_calculerPoids(const Salle* voisin) const {
    if (!voisin->peutAccueillir())
        return -1;

    int cap = voisin->getCapacite();

    if (cap == Salle::CAPACITE_ILLIMITEE)
        return 1;

    if (cap == 1)
        return 1;

    // place_restante = cap - occupants - reserves + partants
    // peutAccueillir() garantit déjà que c'est > 0
    int placeRestante = cap - voisin->getOccupants();
    if (placeRestante <= 0) return -1;

    return cap / placeRestante; // plus c'est plein, plus c'est cher
}

// ==============================
//  _dijkstra — algorithme pur
// ==============================

namespace {
    struct NoeudFile {
        Salle* salle;
        int    distance;
        bool operator>(const NoeudFile& o) const { return distance > o.distance; }
    };
}

ResultatDijkstra AlgorithmeDijkstra::_dijkstra() const {
    ResultatDijkstra res;
    res.cheminTrouve = false;
    res.tempsUs      = 0;
    res.nbTours      = 0;

    const map<string, Salle*>& salles = _fourmiliere->getSalles();
    Salle* depart  = _fourmiliere->getVestibule();
    Salle* arrivee = _fourmiliere->getDortoir();

    if (!depart || !arrivee) return res;

    auto debut = high_resolution_clock::now();

    for (const auto& [nom, salle] : salles) {
        res.distances[nom]     = INT_MAX;
        res.predecesseurs[nom] = "";
    }
    res.distances[depart->getNom()] = 0;

    priority_queue<NoeudFile,
                   vector<NoeudFile>,
                   greater<NoeudFile>> file;
    file.push({depart, 0});

    map<string, bool> visite;
    for (const auto& [nom, salle] : salles)
        visite[nom] = false;

    while (!file.empty()) {
        NoeudFile courant = file.top();
        file.pop();

        const string& nomCourant = courant.salle->getNom();
        if (visite[nomCourant]) continue;
        visite[nomCourant] = true;

        if (courant.salle == arrivee) {
            res.cheminTrouve = true;
            break;
        }

        for (Salle* voisin : courant.salle->getVoisins()) {
            const string& nomVoisin = voisin->getNom();
            if (visite[nomVoisin]) continue;
            if (res.distances[nomCourant] == INT_MAX) continue;

            int poids = _calculerPoids(voisin);
            if (poids == -1) continue; // salle pleine → on saute

            int nouvelleDist = res.distances[nomCourant] + poids;
            if (nouvelleDist < res.distances[nomVoisin]) {
                res.distances[nomVoisin]     = nouvelleDist;
                res.predecesseurs[nomVoisin] = nomCourant;
                file.push({voisin, nouvelleDist});
            }
        }
    }

    auto fin    = high_resolution_clock::now();
    res.tempsUs = duration_cast<microseconds>(fin - debut).count();

    return res;
}

// ============================================================
//  _reconstruireChemin
// ============================================================

vector<Salle*> AlgorithmeDijkstra::_reconstruireChemin(
    const map<string, string>& pred,
    Salle* depart,
    Salle* arrivee) const
{
    vector<Salle*> chemin;
    chemin.reserve(16);

    const map<string, Salle*>& salles = _fourmiliere->getSalles();

    string courant = arrivee->getNom();
    while (!courant.empty()) {
        auto it = salles.find(courant);
        if (it == salles.end()) return {};
        chemin.push_back(it->second);

        auto itPred = pred.find(courant);
        if (itPred == pred.end() || itPred->second.empty()) break;
        courant = itPred->second;
    }

    reverse(chemin.begin(), chemin.end());

    if (chemin.empty() || chemin.front() != depart)
        return {};

    return chemin;
}

// ============================================================
//  _deplacerFourmis
//
//  Recalcule Dijkstra à chaque tour pour adapter le chemin
//  à l'état réel des salles (capacités remplies dynamiquement).
//  Envoie en parallèle autant de fourmis que possible par étape.
// ============================================================

int AlgorithmeDijkstra::_deplacerFourmis(ResultatDijkstra& resGlobal) const {
    const vector<Fourmi*>& fourmis = _fourmiliere->getFourmis();
    Salle* vestibule = _fourmiliere->getVestibule();
    Salle* dortoir   = _fourmiliere->getDortoir();
    int nbFourmis    = (int)fourmis.size();
    int tour         = 0;

    auto nbAuDortoir = [&]() {
        int n = 0;
        for (const Fourmi* f : fourmis)
            if (f->getSalleActuelle() == dortoir) n++;
        return n;
    };

    while (nbAuDortoir() < nbFourmis) {
        tour++;
        bool auMoinsUn = false;

        cout << "  Tour " << tour << " : ";

        // ── Recalcul Dijkstra selon l'état actuel des salles ──────────
        ResultatDijkstra resTour = _dijkstra();
        vector<Salle*> chemin = _reconstruireChemin(
            resTour.predecesseurs, vestibule, dortoir
        );

        if (chemin.empty()) {
            cout << "\n  [WARN] Plus aucun chemin accessible — simulation bloquee.\n";
            break;
        }

        // Met à jour le chemin dans le résultat global (dernier chemin utilisé)
        resGlobal.chemin = chemin;

        // ── Phase 1 : planification (fin -> début du chemin) ──────────
        // On itère de la fin vers le début pour libérer les places
        // avant d'essayer d'y envoyer de nouvelles fourmis.
        for (int etape = (int)chemin.size() - 1; etape >= 1; etape--) {
            Salle* source = chemin[etape - 1];
            Salle* dest   = chemin[etape];

            for (Fourmi* f : fourmis) {
                if (f->getSalleActuelle() != source) continue;

                // planifierDeplacement appelle dest->reserver()
                // et vérifie peutAccueillir() (occupants - partants + reserves < cap)
                // → plusieurs fourmis peuvent réserver la même salle
                //   jusqu'à saturation de sa capacité
                if (f->planifierDeplacement(dest)) {
                    cout << "f" << f->getId()
                         << "(" << source->getNom()
                         << "->" << dest->getNom() << ") ";
                    auMoinsUn = true;
                }
            }
        }

        // ── Phase 2 : commit ──────────────────────────────────────────
        for (Fourmi* f : fourmis)
            f->commitDeplacement();

        cout << "\n";

        if (!auMoinsUn) {
            cout << "  [WARN] Aucun deplacement possible — simulation bloquee.\n";
            break;
        }
    }

    return tour;
}

// ============================================================
//  afficherResultat
// ============================================================

void AlgorithmeDijkstra::afficherResultat(const ResultatDijkstra& res) const {
    cout << "  ----------------------------------------\n";

    if (!res.cheminTrouve || res.chemin.empty()) {
        cout << "  Aucun chemin trouve entre Sv et Sd.\n";
        cout << "  ----------------------------------------\n";
        return;
    }

    cout << "  Chemin optimal : ";
    for (int i = 0; i < (int)res.chemin.size(); i++) {
        cout << res.chemin[i]->getNom();
        if (i < (int)res.chemin.size() - 1) cout << " -> ";
    }
    cout << "\n";

    const string& nomDortoir = _fourmiliere->getDortoir()->getNom();
    auto it = res.distances.find(nomDortoir);
    if (it != res.distances.end() && it->second != INT_MAX)
        cout << "  Distance       : " << it->second << " (poids cumule)\n";

    cout << "  Distances depuis Sv :\n";
    for (const auto& [nom, dist] : res.distances) {
        cout << "    " << nom << " : ";
        if (dist == INT_MAX) cout << "inaccessible";
        else                 cout << dist;
        cout << "\n";
    }

    cout << "  Tours simules  : " << res.nbTours << "\n";
    cout << "  Temps algo     : " << res.tempsUs << " us\n";

    cout << "  Fourmis (id -> salle finale) :\n";
    for (const Fourmi* f : _fourmiliere->getFourmis()) {
        cout << "    f" << f->getId()
             << " -> " << f->getSalleActuelle()->getNom() << "\n";
    }

    cout << "  ----------------------------------------\n";
}