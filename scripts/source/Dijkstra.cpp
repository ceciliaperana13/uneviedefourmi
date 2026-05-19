// algo de Dijkstra
#include "Dijkstra.hpp"
#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>

using namespace std;


//  Structures


struct Arete {
    int destination;
    int poids;
};

struct Noeud {
    int id;
    int distance;
    bool operator>(const Noeud& autre) const {
        return distance > autre.distance;
    }
};


//arrete de Dijkstra
vector<int> dijkstra(
    const vector<vector<Arete>>& graphe,
    int nbSommets,
    int depart,
    int arrivee,
    vector<int>& predecesseurs)
{
    vector<int> distances(nbSommets, INT_MAX);
    vector<bool> visite(nbSommets, false);
    predecesseurs.assign(nbSommets, -1);

    distances[depart] = 0;

    // Min-heap : (distance, id)
    priority_queue<Noeud, vector<Noeud>, greater<Noeud>> file;
    file.push({depart, 0});

    while (!file.empty()) {
        Noeud courant = file.top();
        file.pop();

        if (visite[courant.id]) continue;
        visite[courant.id] = true;

        if (courant.id == arrivee) break;

        for (const Arete& arete : graphe[courant.id]) {
            if (visite[arete.destination]) continue;

            int nouvelleDist = distances[courant.id] + arete.poids;
            if (nouvelleDist < distances[arete.destination]) {
                distances[arete.destination] = nouvelleDist;
                predecesseurs[arete.destination] = courant.id;
                file.push({arete.destination, nouvelleDist});
            }
        }
    }

    return distances;
}


//  Reconstruction du chemin depuis les predecesseurs


vector<int> reconstruireChemin(const vector<int>& predecesseurs, int depart, int arrivee) {
    vector<int> chemin;
    for (int n = arrivee; n != -1; n = predecesseurs[n])
        chemin.push_back(n);
    reverse(chemin.begin(), chemin.end());
    if (chemin.front() != depart) return {}; // pas de chemin
    return chemin;
}



 

int main() {
    // Graphe à 6 sommets : A=0  B=1  C=2  D=3  E=4  F=5
    int nbSommets = 6;
    vector<vector<Arete>> graphe(nbSommets);

    // Arêtes non orientées (on ajoute dans les deux sens)
    auto ajouterArete = [&](int u, int v, int poids) {
        graphe[u].push_back({v, poids});
        graphe[v].push_back({u, poids});
    };

    ajouterArete(0, 1, 4);  // A-B
    ajouterArete(0, 2, 2);  // A-C
    ajouterArete(1, 2, 5);  // B-C
    ajouterArete(1, 3, 10); // B-D
    ajouterArete(2, 4, 3);  // C-E
    ajouterArete(4, 3, 4);  // E-D
    ajouterArete(3, 5, 11); // D-F
    ajouterArete(4, 5, 6);  // E-F

    int depart  = 0; // A
    int arrivee = 5; // F

    vector<int> predecesseurs;
    vector<int> distances = dijkstra(graphe, nbSommets, depart, arrivee, predecesseurs);

    // Affichage
    vector<string> noms = {"A", "B", "C", "D", "E", "F"};

    cout << "Distance minimale de " << noms[depart]
         << " a " << noms[arrivee] << " : ";

    if (distances[arrivee] == INT_MAX) {
        cout << "inaccessible" << endl;
        return 0;
    }

    cout << distances[arrivee] << endl;

    vector<int> chemin = reconstruireChemin(predecesseurs, depart, arrivee);
    cout << "Chemin : ";
    for (int i = 0; i < (int)chemin.size(); i++) {
        cout << noms[chemin[i]];
        if (i < (int)chemin.size() - 1) cout << " -> ";
    }
    cout << endl;

    return 0;
}