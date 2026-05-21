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
    vector<Salle*>      chemin;        // chemin Sv -> Sd 
    long long           tempsUs;       // durée de l'algo en microsecondes
    int                 nbTours;       // nombre de tours pour tout déplacer
    bool                cheminTrouve;
};

// ============================================================
//  AlgorithmeDijkstra
// ============================================================

class AlgorithmeDijkstra {
public:
    explicit AlgorithmeDijkstra(const Fourmiliere* fourmiliere);
    ~AlgorithmeDijkstra() = default;

    AlgorithmeDijkstra(const AlgorithmeDijkstra&)            = delete;
    AlgorithmeDijkstra& operator=(const AlgorithmeDijkstra&) = delete;
    AlgorithmeDijkstra(AlgorithmeDijkstra&&)                 = default;
    AlgorithmeDijkstra& operator=(AlgorithmeDijkstra&&)      = default;

    ResultatDijkstra executer();
    void afficherResultat(const ResultatDijkstra& res) const;

private:
    ResultatDijkstra _dijkstra() const;

    vector<Salle*> _reconstruireChemin(
        const map<string, string>& pred,
        Salle* depart,
        Salle* arrivee) const;

    // Simule les fourmis étape par étape le long du chemin
    int _deplacerFourmis(const vector<Salle*>& chemin) const;

    const Fourmiliere* _fourmiliere; // non-owning
};