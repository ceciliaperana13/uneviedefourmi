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
    ResultatDijkstra res = _dijkstra();
    res.nbTours = 0;

    res.chemin = _reconstruireChemin(
        res.predecesseurs,
        _fourmiliere->getVestibule(),
        _fourmiliere->getDortoir()
    );

    if (!res.chemin.empty())
        res.nbTours = _deplacerFourmis(res.chemin);

    return res;
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

        const int POIDS = 1;

        for (Salle* voisin : courant.salle->getVoisins()) {
            const string& nomVoisin = voisin->getNom();
            if (visite[nomVoisin]) continue;
            if (res.distances[nomCourant] == INT_MAX) continue;

            int nouvelleDist = res.distances[nomCourant] + POIDS;
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
//  Simule le déplacement des fourmis tour par tour le long
//  du chemin optimal trouvé par Dijkstra.
//
//  Chaque tour :
//    - On parcourt le chemin DE LA FIN VERS LE DÉBUT
//      (pipeline) pour éviter qu'une fourmi avance deux
//      fois dans le même tour.
//    - Phase 1 : planifierDeplacement() pour toutes les
//      fourmis éligibles (respecte les capacités).
//    - Phase 2 : commitDeplacement() pour valider.
//
//  On boucle jusqu'à ce que toutes les fourmis soient
//  au dortoir, ou qu'aucun déplacement ne soit possible
//  (cas bloqué — sécurité anti boucle infinie).
// ============================================================

int AlgorithmeDijkstra::_deplacerFourmis(const vector<Salle*>& chemin) const {
    const vector<Fourmi*>& fourmis = _fourmiliere->getFourmis();
    Salle* dortoir = chemin.back();
    int nbFourmis  = (int)fourmis.size();
    int tour       = 0;

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

        // ---- Phase 1 : planification (fin -> début du chemin) ----
        for (int etape = (int)chemin.size() - 1; etape >= 1; etape--) {
            Salle* source = chemin[etape - 1];
            Salle* dest   = chemin[etape];

            for (Fourmi* f : fourmis) {
                if (f->getSalleActuelle() == source) {
                    if (f->planifierDeplacement(dest)) {
                        cout << "f" << f->getId()
                             << "(" << source->getNom()
                             << "->" << dest->getNom() << ") ";
                        auMoinsUn = true;
                    }
                }
            }
        }

        // ---- Phase 2 : commit ----
        for (Fourmi* f : fourmis)
            f->commitDeplacement();

        cout << "\n";

        // Sécurité : si rien n'a bougé, on est bloqué
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

    // Chemin optimal
    cout << "  Chemin optimal : ";
    for (int i = 0; i < (int)res.chemin.size(); i++) {
        cout << res.chemin[i]->getNom();
        if (i < (int)res.chemin.size() - 1) cout << " -> ";
    }
    cout << "\n";

    // Distance
    const string& nomDortoir = _fourmiliere->getDortoir()->getNom();
    auto it = res.distances.find(nomDortoir);
    if (it != res.distances.end() && it->second != INT_MAX)
        cout << "  Distance       : " << it->second << " tunnel(s)\n";

    // Distances depuis Sv
    cout << "  Distances depuis Sv :\n";
    for (const auto& [nom, dist] : res.distances) {
        cout << "    " << nom << " : ";
        if (dist == INT_MAX) cout << "inaccessible";
        else                 cout << dist;
        cout << "\n";
    }

    // Nombre de tours
    cout << "  Tours simulés  : " << res.nbTours << "\n";

    // Temps d'exécution de l'algo pur
    cout << "  Temps algo     : " << res.tempsUs << " us\n";

    // État final des fourmis
    cout << "  Fourmis (id -> salle finale) :\n";
    for (const Fourmi* f : _fourmiliere->getFourmis()) {
        cout << "    f" << f->getId()
             << " -> " << f->getSalleActuelle()->getNom() << "\n";
    }

    cout << "  ----------------------------------------\n";
}