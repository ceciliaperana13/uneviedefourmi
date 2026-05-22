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

struct ResultatDijkstra {
    map<string, int>    distances;
    map<string, string> predecesseurs;
    vector<Salle*>      chemin;       // chemin initial Sv -> Sd
    long long           tempsUs;
    int                 nbTours;
    bool                cheminTrouve;
};

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

    int _calculerPoids(const Salle* voisin) const;
    int _deplacerFourmis(ResultatDijkstra& res) const;

    const Fourmiliere* _fourmiliere;
};