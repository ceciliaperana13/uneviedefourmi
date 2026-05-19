#include "../include/Dijkstra.hpp"

#include <queue>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <functional>

using namespace std;
using namespace std::chrono;


//  Constructeur
AlgorithmeDijkstra::AlgorithmeDijkstra(const Fourmiliere* fourmiliere)
    : _fourmiliere(fourmiliere)
{
    if (!fourmiliere)
        throw invalid_argument("AlgorithmeDijkstra : pointeur fourmiliere nul");
}


//  executer — point d'entrée public
ResultatDijkstra AlgorithmeDijkstra::executer() {
    ResultatDijkstra res = _dijkstra();
    res.chemin = _reconstruireChemin(
        res.predecesseurs,
        _fourmiliere->getVestibule(),
        _fourmiliere->getDortoir()
    );

    if (!res.chemin.empty())
        _deplacerFourmis(res.chemin.back());

    return res;
}


//  _dijkstra — algorithme pur sur les Salle* du graphe
// Noeud interne pour la file de priorité (local à ce fichier)
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

    const map<string, Salle*>& salles = _fourmiliere->getSalles();
    Salle* depart  = _fourmiliere->getVestibule();
    Salle* arrivee = _fourmiliere->getDortoir();

    if (!depart || !arrivee)
        return res;

    // ---- Chrono démarré le plus tôt possible ----
    auto debut = high_resolution_clock::now();

    // Initialisation : toutes les distances à INT_MAX
    for (const auto& [nom, salle] : salles) {
        res.distances[nom]     = INT_MAX;
        res.predecesseurs[nom] = "";
    }
    res.distances[depart->getNom()] = 0;

    // Min-heap
    priority_queue<NoeudFile,
                   vector<NoeudFile>,
                   greater<NoeudFile>> file;
    file.push({depart, 0});

    map<string, bool> visite;
    for (const auto& [nom, _] : salles)
        visite[nom] = false;

    while (!file.empty()) {
        NoeudFile courant = file.top();
        file.pop();

        const string& nomCourant = courant.salle->getNom();

        if (visite[nomCourant]) continue;
        visite[nomCourant] = true;

        // Arrivée atteinte
        if (courant.salle == arrivee) {
            res.cheminTrouve = true;
            break;
        }

        // Poids = pour chaque tunel et egale a la salle ou les fourmis vont se deplacer 
        // Remplacer par salle->getPoids() si les tunnels ont un poids variable
        const int POIDS_TUNNEL = 1;

        for (Salle* voisin : courant.salle->getVoisins()) {
            const string& nomVoisin = voisin->getNom();

            if (visite[nomVoisin]) continue;

            // Protection contre le débordement INT_MAX + poids
            if (res.distances[nomCourant] == INT_MAX) continue;

            int nouvelleDist = res.distances[nomCourant] + POIDS_TUNNEL;
            if (nouvelleDist < res.distances[nomVoisin]) {
                res.distances[nomVoisin]     = nouvelleDist;
                res.predecesseurs[nomVoisin] = nomCourant;
                file.push({voisin, nouvelleDist});
            }
        }
    }

    // ---- Chrono arrêté dès la fin du calcul ----
    auto fin = high_resolution_clock::now();
    res.tempsUs = duration_cast<microseconds>(fin - debut).count();

    return res;
}


//  _reconstruireChemin
vector<Salle*> AlgorithmeDijkstra::_reconstruireChemin(
    const map<string, string>& pred,
    Salle* depart,
    Salle* arrivee) const
{
    vector<Salle*> chemin;
    chemin.reserve(16);

    const map<string, Salle*>& salles = _fourmiliere->getSalles();

    // Remonte depuis l'arrivée jusqu'au départ via les prédécesseurs
    string courant = arrivee->getNom();
    while (!courant.empty()) {
        auto it = salles.find(courant);
        if (it == salles.end()) return {}; // salle introuvable

        chemin.push_back(it->second);

        auto itPred = pred.find(courant);
        if (itPred == pred.end() || itPred->second.empty()) break;
        courant = itPred->second;
    }

    reverse(chemin.begin(), chemin.end());

    // Vérifie que le chemin part bien du départ
    if (chemin.empty() || chemin.front() != depart)
        return {};

    return chemin;
}


//  _deplacerFourmis — déplace chaque Fourmi* vers la destination
void AlgorithmeDijkstra::_deplacerFourmis(Salle* destination) const {
    for (Fourmi* f : _fourmiliere->getFourmis())
        f->setSalle(destination); // adapte au nom exact de votre setter
}


//  afficherResultat


void AlgorithmeDijkstra::afficherResultat(const ResultatDijkstra& res) const {
    cout << "=============================\n";

    if (!res.cheminTrouve || res.chemin.empty()) {
        cout << "  Aucun chemin trouve entre Sv et Sd.\n";
        cout << "================================\n";
        return;
    }

    // Chemin optimal
    cout << "  Chemin optimal : ";
    for (int i = 0; i < (int)res.chemin.size(); i++) {
        cout << res.chemin[i]->getNom();
        if (i < (int)res.chemin.size() - 1) cout << " -> ";
    }
    cout << "\n";

    // Distance totale
    cout << "  Distance       : "
         << res.distances.at(_fourmiliere->getDortoir()->getNom()) << "\n";

    // Toutes les distances depuis Sv
    cout << "  Distances depuis Sv :\n";
    for (const auto& [nom, dist] : res.distances) {
        cout << "    " << nom << " : ";
        if (dist == INT_MAX) cout << "inaccessible";
        else                 cout << dist;
        cout << "\n";
    }

    // Temps d'exécution
    cout << "  Temps          : " << res.tempsUs << " us\n";

    // Position des fourmis après déplacement
    cout << "  Fourmis        : ";
    for (Fourmi* f : _fourmiliere->getFourmis())
        cout << f->getNom() << " ";
    cout << "\n";

    cout << "========================================\n";
}