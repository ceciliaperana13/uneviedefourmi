#pragma once

#include <vector>
#include <climits>


//  Structures
struct Arete {
    int destination;
    int poids;
};

struct Noeud {
    int id;
    int distance;
    bool operator>(const Noeud& autre) const;
};




//  Fonctions
std::vector<int> dijkstra(
    const std::vector<std::vector<Arete>>& graphe,
    int nbSommets,//nombre total de sommets
    int depart,//indice du sommet source
    int arrivee,//indice du sommet cible
    std::vector<int>& predecesseurs
);

// Reconstruction du chemin depuis les predecesseurs
// Retourne un vecteur vide si aucun chemin n'existe
std::vector<int> reconstruireChemin(
    const std::vector<int>& predecesseurs,
    int depart,
    int arrivee
);