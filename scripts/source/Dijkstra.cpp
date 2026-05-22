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
//  executer
// ====================================

ResultatDijkstra AlgorithmeDijkstra::executer() {
    ResultatDijkstra res = _dijkstra();
    res.nbTours = 0;

    res.chemin = _reconstruireChemin(
        res.predecesseurs,
        _fourmiliere->getVestibule(),
        _fourmiliere->getDortoir()
    );

    if (res.cheminTrouve)
        res.nbTours = _deplacerFourmis(res);

    return res;
}

// ============================================================
//  _calculerPoids
//  Retourne -1 si inaccessible, sinon le poids de la salle
// ============================================================

int AlgorithmeDijkstra::_calculerPoids(const Salle* voisin) const {
    if (!voisin->peutAccueillir())
        return -1;

    int cap = voisin->getCapacite();

    if (cap == Salle::CAPACITE_ILLIMITEE)
        return 1;

    if (cap == 1)
        return 1;

    int placeRestante = cap - voisin->getOccupants();
    if (placeRestante <= 0) return -1;

    return cap / placeRestante;
}

// ==============================
//  _dijkstra
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

    priority_queue<NoeudFile, vector<NoeudFile>, greater<NoeudFile>> file;
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
            if (poids == -1) continue;

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
//  Principe pipeline :
//  - Chaque fourmi encore à Sv reçoit un chemin calculé au
//    moment où une place se libère sur le chemin optimal.
//  - Les fourmis déjà en route suivent leur prochain pas
//    sur leur chemin assigné.
//  - On remplit les capacités en parallèle à chaque tour.
// ============================================================

int AlgorithmeDijkstra::_deplacerFourmis(ResultatDijkstra& resGlobal) const {
    const vector<Fourmi*>& fourmis   = _fourmiliere->getFourmis();
    Salle*                 vestibule = _fourmiliere->getVestibule();
    Salle*                 dortoir   = _fourmiliere->getDortoir();
    int                    nbFourmis = (int)fourmis.size();
    int                    tour      = 0;

    // Chemin assigné à chaque fourmi (vide = pas encore assigné)
    map<int, vector<Salle*>> cheminParFourmi;
    // Étape courante de chaque fourmi sur son chemin (index de la salle suivante)
    map<int, int> etapeParFourmi;

    for (Fourmi* f : fourmis) {
        cheminParFourmi[f->getId()] = {};
        etapeParFourmi [f->getId()] = 0;
    }

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

        // ── Recalcul Dijkstra sur l'état actuel ───────────────────────
        // Sert à assigner un chemin aux fourmis encore à Sv
        // ET à mettre à jour resGlobal avec le chemin courant optimal
        ResultatDijkstra resTour = _dijkstra();
        vector<Salle*> cheminOptimal = _reconstruireChemin(
            resTour.predecesseurs, vestibule, dortoir
        );
        if (!cheminOptimal.empty())
            resGlobal.chemin = cheminOptimal;

        // ── Assigner un chemin aux fourmis à Sv sans chemin ───────────
        for (Fourmi* f : fourmis) {
            if (f->getSalleActuelle() != vestibule) continue;
            if (!cheminParFourmi[f->getId()].empty()) continue;

            // Recalcul dédié depuis Sv pour cette fourmi
            ResultatDijkstra rF = _dijkstra();
            vector<Salle*> ch  = _reconstruireChemin(
                rF.predecesseurs, vestibule, dortoir
            );
            if (!ch.empty()) {
                cheminParFourmi[f->getId()] = ch;
                etapeParFourmi [f->getId()] = 1; // prochain pas = index 1
            }
        }

        // ── Phase 1 : planification (fin -> début pour libérer d'abord)
        // On trie les fourmis par position décroissante sur leur chemin
        // pour que celles qui sont le plus avancées bougent en premier
        // et libèrent leurs places avant celles qui sont derrière.
        vector<Fourmi*> ordre(fourmis.begin(), fourmis.end());
        sort(ordre.begin(), ordre.end(), [&](Fourmi* a, Fourmi* b) {
            return etapeParFourmi[a->getId()] > etapeParFourmi[b->getId()];
        });

        for (Fourmi* f : ordre) {
            if (f->getSalleActuelle() == dortoir) continue;

            vector<Salle*>& ch = cheminParFourmi[f->getId()];
            int&            ep = etapeParFourmi [f->getId()];

            if (ch.empty() || ep >= (int)ch.size()) continue;

            Salle* dest = ch[ep];

            if (f->planifierDeplacement(dest)) {
                cout << "f" << f->getId()
                     << "(" << f->getSalleActuelle()->getNom()
                     << "->" << dest->getNom() << ") ";
                auMoinsUn = true;
                ep++; // avancer l'index pour le prochain tour
            } else {
                // Destination pleine → recalculer un détour depuis la position actuelle
                ResultatDijkstra rDetour = _dijkstra();
                vector<Salle*> detour = _reconstruireChemin(
                    rDetour.predecesseurs,
                    f->getSalleActuelle(),
                    dortoir
                );
                if (!detour.empty()) {
                    ch = detour;
                    ep = 1;
                    // Retenter immédiatement
                    if (f->planifierDeplacement(ch[0] == f->getSalleActuelle()
                                                 ? ch[1] : ch[ep - 1])) {
                        // on ne réessaie pas ici, sera tenté au prochain tour
                    }
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