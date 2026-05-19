#include "Dijkstra.hpp"

#include <queue>
#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace std::chrono;


//  Fourmi
Fourmi::Fourmi(int id, string nom)
    : _id(id), _nom(nom.empty() ? "f" + to_string(id) : nom), _posActuelle(-1)
{}

int         Fourmi::getId()  const { return _id; }
string      Fourmi::getNom() const { return _nom; }
int         Fourmi::getPos() const { return _posActuelle; }
void        Fourmi::setPos(int sommet) { _posActuelle = sommet; }

// ============================================================
//  AlgorithmeDijkstra — construction
// ============================================================

AlgorithmeDijkstra::AlgorithmeDijkstra(int nbSommets)
    : _nbSommets(nbSommets), _graphe(nbSommets)
{}

// ---- Graphe ----
void AlgorithmeDijkstra::ajouterArete(int source, int dest, int poids) {
    _graphe[source].push_back({dest, poids});
}

void AlgorithmeDijkstra::ajouterAreteNonOrientee(int source, int dest, int poids) {
    _graphe[source].push_back({dest, poids});
    _graphe[dest].push_back({source, poids});
}

// ---- Fourmis ----
void AlgorithmeDijkstra::ajouterFourmi(unique_ptr<Fourmi> fourmi) {
    _fourmis.push_back(move(fourmi));
}

int AlgorithmeDijkstra::nbSommets() const { return _nbSommets; }
int AlgorithmeDijkstra::nbFourmis() const { return static_cast<int>(_fourmis.size()); }

const Fourmi* AlgorithmeDijkstra::getFourmi(int index) const {
    if (index < 0 || index >= (int)_fourmis.size())
        throw out_of_range("Index fourmi invalide");
    return _fourmis[index].get(); // pointeur observateur, pas de transfert
}


//  Libération mémoire explicite
void AlgorithmeDijkstra::liberer() {
    for (auto& voisins : _graphe) {
        voisins.clear();
        voisins.shrink_to_fit(); // rend la mémoire au système
    }
    _graphe.clear();
    _graphe.shrink_to_fit();

    _fourmis.clear();            // unique_ptr -> destructeurs appelés automatiquement
    _fourmis.shrink_to_fit();

    _nbSommets = 0;
}


//  Algorithme Dijkstra (cœur)
// Noeud interne pour la file de priorité — défini localement pour ne pas polluer le .hpp
struct NoeudInterne {
    int id;
    int distance;
    bool operator>(const NoeudInterne& o) const { return distance > o.distance; }
};

ResultatDijkstra AlgorithmeDijkstra::_dijkstra(int depart, int arrivee) const {
    ResultatDijkstra res;
    res.cheminTrouve = false;
    res.tempsUs      = 0;

    // Chrono démarré le plus tôt possible 
    auto debut = high_resolution_clock::now();

    // Réservation groupée pour éviter les réallocations
    res.distances.assign(_nbSommets, INT_MAX);
    res.predecesseurs.assign(_nbSommets, -1);
    vector<bool> visite(_nbSommets, false);

    res.distances[depart] = 0;

    priority_queue<NoeudInterne,
                   vector<NoeudInterne>,
                   greater<NoeudInterne>> file;
    file.push({depart, 0});

    while (!file.empty()) {
        NoeudInterne courant = file.top();
        file.pop();

        if (visite[courant.id]) continue;
        visite[courant.id] = true;

        if (courant.id == arrivee) {
            res.cheminTrouve = true;
            break;
        }

        for (const Arete& arete : _graphe[courant.id]) {
            if (visite[arete.destination]) continue;

            // Protection contre le débordement INT_MAX + poids
            if (res.distances[courant.id] == INT_MAX) continue;

            int nouvelleDist = res.distances[courant.id] + arete.poids;//arret + poid 
            if (nouvelleDist < res.distances[arete.destination]) {
                res.distances[arete.destination]     = nouvelleDist;
                res.predecesseurs[arete.destination] = courant.id;
                file.push({arete.destination, nouvelleDist});
            }
        }
    }

    // ---- Chrono arrêté dès la fin du calcul ----
    auto fin = high_resolution_clock::now();
    res.tempsUs = duration_cast<microseconds>(fin - debut).count();

    return res;
}


//  Reconstruction du chemin
vector<int> AlgorithmeDijkstra::_reconstruireChemin(
    const vector<int>& pred, int depart, int arrivee) const
{
    vector<int> chemin;
    chemin.reserve(16); // pré-alloue pour éviter les réallocations

    for (int n = arrivee; n != -1; n = pred[n])
        chemin.push_back(n);

    reverse(chemin.begin(), chemin.end());

    if (chemin.empty() || chemin.front() != depart)
        return {};

    return chemin;
}


//  executer — lance l'algo + déplace les fourmis

ResultatDijkstra AlgorithmeDijkstra::executer(int depart, int arrivee) {
    ResultatDijkstra res = _dijkstra(depart, arrivee);
    res.chemin = _reconstruireChemin(res.predecesseurs, depart, arrivee);

    if (!res.chemin.empty()) {
        // Déplace chaque fourmi sur l'arrivée du chemin optimal
        int destination = res.chemin.back();
        for (auto& f : _fourmis)
            f->setPos(destination);
    }

    return res;
}


//  Affichage
void AlgorithmeDijkstra::afficherResultat(
    const ResultatDijkstra& res,
    const vector<string>& noms) const
{
    auto nomSommet = [&](int i) -> string {
        if (i >= 0 && i < (int)noms.size()) return noms[i];
        return to_string(i);
    };

    cout << "========================================\n";

    if (!res.cheminTrouve || res.chemin.empty()) {
        cout << "  Aucun chemin trouve.\n";
        cout << "========================================\n";
        return;
    }

    cout << "  Distance minimale : " << res.distances[res.chemin.back()] << "\n";
    cout << "  Chemin optimal    : ";
    for (int i = 0; i < (int)res.chemin.size(); i++) {
        cout << nomSommet(res.chemin[i]);
        if (i < (int)res.chemin.size() - 1) cout << " -> ";
    }
    cout << "\n";

    cout << "  Temps d'execution : " << res.tempsUs << " us\n";

    if (!_fourmis.empty()) {
        cout << "  Fourmis arrivees  : ";
        for (int i = 0; i < (int)_fourmis.size(); i++) {
            cout << _fourmis[i]->getNom()
                 << " (pos=" << nomSommet(_fourmis[i]->getPos()) << ")";
            if (i < (int)_fourmis.size() - 1) cout << ", ";
        }
        cout << "\n";
    }

    cout << "========================================\n";
}