#pragma once

#include <vector>
#include <memory>
#include <string>
#include <climits>
#include <chrono>


class Fourmi {
public:
    explicit Fourmi(int id, std::string nom = "");
    ~Fourmi() = default;

    int         getId()  const;
    std::string getNom() const;
    int         getPos() const;
    void        setPos(int sommet);

private:
    int         _id;
    std::string _nom;
    int         _posActuelle;
};


//  Arete — arc pondéré du graphe


struct Arete {
    int destination;
    int poids;
};


//  ResultatDijkstra — résultat retourné par l'algorithme


struct ResultatDijkstra {
    std::vector<int> distances;     // distance minimale depuis depart
    std::vector<int> predecesseurs; // pour reconstruire le chemin
    std::vector<int> chemin;        // chemin optimal depart -> arrivee
    long long        tempsUs;       // temps d'exécution en microsecondes
    bool             cheminTrouve;
};


//  AlgorithmeDijkstra

class AlgorithmeDijkstra {
public:
    explicit AlgorithmeDijkstra(int nbSommets);
    ~AlgorithmeDijkstra() = default;

    // Copie interdite (graphe potentiellement lourd)
    AlgorithmeDijkstra(const AlgorithmeDijkstra&)            = delete;
    AlgorithmeDijkstra& operator=(const AlgorithmeDijkstra&) = delete;

    // Déplacement autorisé
    AlgorithmeDijkstra(AlgorithmeDijkstra&&)            = default;
    AlgorithmeDijkstra& operator=(AlgorithmeDijkstra&&) = default;

    // ---- Construction du graphe ----
    void ajouterArete(int source, int dest, int poids);
    void ajouterAreteNonOrientee(int source, int dest, int poids);

    // ---- Fourmis (propriété transférée via unique_ptr) ----
    void ajouterFourmi(std::unique_ptr<Fourmi> fourmi);

    // ---- Exécution ----
    // Lance Dijkstra et déplace toutes les fourmis sur le chemin optimal
    ResultatDijkstra executer(int depart, int arrivee);

    // ---- Accesseurs ----
    int           nbSommets() const;
    int           nbFourmis() const;
    const Fourmi* getFourmi(int index) const; // pointeur observateur (non-owning)

    // Affiche le résultat (noms optionnels des sommets)
    void afficherResultat(const ResultatDijkstra& res,
                          const std::vector<std::string>& noms = {}) const;

    // Libère graphe + fourmis et vide la RAM
    void liberer();

private:
    ResultatDijkstra _dijkstra(int depart, int arrivee) const;
    std::vector<int> _reconstruireChemin(const std::vector<int>& pred,
                                          int depart, int arrivee) const;

    int                                  _nbSommets;
    std::vector<std::vector<Arete>>      _graphe;
    std::vector<std::unique_ptr<Fourmi>> _fourmis; // propriété exclusive
};