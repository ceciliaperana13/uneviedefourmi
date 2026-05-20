#pragma once

#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <climits>

#include "salle.hpp"
#include "fourmi.hpp"
#include "fourmiliere.hpp"

using namespace std;

// ============================================================
//  ResultatDijkstra
// ============================================================

struct ResultatDijkstra {
    map<string, int>    distances;     // distance minimale par nom de salle
    map<string, string> predecesseurs; // nom -> nom prédécesseur
    vector<Salle*>      chemin;        // chemin Sv -> Sd (pointeurs non-owning)
    long long           tempsUs;       // durée de l'algo en microsecondes
    bool                cheminTrouve;
};

// ============================================================
//  AlgorithmeDijkstra
// ============================================================

class AlgorithmeDijkstra {
public:
    // Pointeur observateur — la Fourmiliere reste propriétaire de ses données
    explicit AlgorithmeDijkstra(const Fourmiliere* fourmiliere);
    ~AlgorithmeDijkstra() = default;

    // Copie interdite
    AlgorithmeDijkstra(const AlgorithmeDijkstra&)            = delete;
    AlgorithmeDijkstra& operator=(const AlgorithmeDijkstra&) = delete;

    // Déplacement autorisé
    AlgorithmeDijkstra(AlgorithmeDijkstra&&)            = default;
    AlgorithmeDijkstra& operator=(AlgorithmeDijkstra&&) = default;

    // Lance Dijkstra et déplace toutes les fourmis via seDeplacer()
    ResultatDijkstra executer();

    // Affiche chemin, distances, temps et position des fourmis
    void afficherResultat(const ResultatDijkstra& res) const;

private:
    ResultatDijkstra _dijkstra() const;

    vector<Salle*>   _reconstruireChemin(
                         const map<string, string>& pred,
                         Salle* depart,
                         Salle* arrivee) const;

    // Appelle f->seDeplacer(destination) pour chaque fourmi
    void             _deplacerFourmis(Salle* destination) const;

    const Fourmiliere* _fourmiliere; // non-owning
};