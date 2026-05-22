#include "../include/Dijkstra.hpp"

#include <queue>
#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace std::chrono;

AlgorithmeDijkstra::AlgorithmeDijkstra(const Fourmiliere* fourmiliere)
    : _fourmiliere(fourmiliere)
{
    if (!fourmiliere)
        throw invalid_argument("AlgorithmeDijkstra : pointeur fourmiliere nul");
}

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
//  Sv et Sd : toujours accessibles (poids 1)
//  Salle pleine (dispo <= 0) : inaccessible (-1)
//  Sinon poids 1
// ============================================================
int AlgorithmeDijkstra::_calculerPoids(const Salle* voisin) const {
    if (voisin->estVestibule() || voisin->estDortoir())
        return 1;

    int cap = voisin->getCapacite();
    if (cap == Salle::CAPACITE_ILLIMITEE)
        return 1;

    int dispo = cap - voisin->getOccupants();
    if (dispo <= 0) return -1;

    return 1;
}

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
        NoeudFile courant = file.top(); file.pop();
        const string& nomCourant = courant.salle->getNom();
        if (visite[nomCourant]) continue;
        visite[nomCourant] = true;

        if (courant.salle == arrivee) { res.cheminTrouve = true; break; }

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

    res.tempsUs = duration_cast<microseconds>(high_resolution_clock::now() - debut).count();
    return res;
}

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
    if (chemin.empty() || chemin.front() != depart) return {};
    return chemin;
}

// ============================================================
//  _deplacerFourmis
//
//  Pipeline :
//  - ep == -1 : fourmi encore a Sv, pas injectee
//  - ep ==  1 : prochaine destination = chemin[1] (premiere apres Sv)
//  - ep ==  k : prochaine destination = chemin[k]
//  - ep >= longueur : fourmi arrivee a Sd
//
//  A chaque tour :
//  1. Recalcule le chemin (les salles pleine peuvent bloquer)
//  2. Les fourmis les plus avancees bougent en premier
//  3. On injecte autant de nouvelles fourmis que possible
// ============================================================
int AlgorithmeDijkstra::_deplacerFourmis(ResultatDijkstra& resGlobal) const {

    const vector<Fourmi*>& fourmis = _fourmiliere->getFourmis();
    Salle* vestibule = _fourmiliere->getVestibule();
    Salle* dortoir   = _fourmiliere->getDortoir();

    int nbFourmis = (int)fourmis.size();
    int tour      = 0;

    // ep = -1 : pas encore injectee ; ep = k : prochaine = chemin[k]
    map<int, int> etapeParFourmi;
    for (Fourmi* f : fourmis)
        etapeParFourmi[f->getId()] = -1;

    vector<vector<MouvementEtape>> etapes;

    auto nbAuDortoir = [&]() {
        int n = 0;
        for (const Fourmi* f : fourmis)
            if (f->getSalleActuelle() == dortoir) n++;
        return n;
    };

    vector<Salle*> chemin = resGlobal.chemin;
    int securite = nbFourmis * (int)chemin.size() * 4 + 20;

    while (nbAuDortoir() < nbFourmis && tour < securite) {
        tour++;

        // Recalcul chemin a chaque tour selon etat courant
        ResultatDijkstra resTour = _dijkstra();
        vector<Salle*> nouveauChemin =
            _reconstruireChemin(resTour.predecesseurs, vestibule, dortoir);
        if (!nouveauChemin.empty())
            chemin = nouveauChemin;

        if (chemin.empty()) break;
        resGlobal.chemin = chemin;
        int longueur = (int)chemin.size();

        // Les plus avancees bougent en premier pour liberer les places
        vector<Fourmi*> ordre(fourmis.begin(), fourmis.end());
        sort(ordre.begin(), ordre.end(), [&](Fourmi* a, Fourmi* b) {
            return etapeParFourmi[a->getId()] > etapeParFourmi[b->getId()];
        });

        vector<MouvementEtape> mouvementsTour;
        bool auMoinsUn = false;

        for (Fourmi* f : ordre) {
            if (f->getSalleActuelle() == dortoir) continue;

            int& ep = etapeParFourmi[f->getId()];

            // Fourmi pas encore partie (toujours a Sv)
            if (ep == -1) {
                if (longueur < 2) continue;
                Salle* dest = chemin[1];
                if (f->planifierDeplacement(dest)) {
                    mouvementsTour.push_back({ f->getId(), dest->getNom() });
                    ep = 2; // apres commit elle sera en chemin[1], prochaine = chemin[2]
                    auMoinsUn = true;
                }
                continue;
            }

            // Fourmi en route
            if (ep >= longueur) continue;
            Salle* dest = chemin[ep];
            if (f->planifierDeplacement(dest)) {
                mouvementsTour.push_back({ f->getId(), dest->getNom() });
                ep++;
                auMoinsUn = true;
            }
        }

        for (Fourmi* f : fourmis)
            f->commitDeplacement();

        if (!mouvementsTour.empty())
            etapes.push_back(mouvementsTour);

        if (!auMoinsUn) break;
    }

    resGlobal.etapes = etapes;
    return tour;
}

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