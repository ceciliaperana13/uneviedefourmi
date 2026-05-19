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


//  ResultatDijkstra — résultat retourné par l'algorithme
struct ResultatDijkstra {
    map<string, int>    distances;     // distance minimale par nom de salle
    map<string, string> predecesseurs; // nom salle -> nom salle précédente
    vector<Salle*>      chemin;        // chemin optimal Sv -> Sd (pointeurs non-owning)
    long long           tempsUs;       // temps d'exécution en microsecondes
    bool                cheminTrouve;
};


//  AlgorithmeDijkstra
class AlgorithmeDijkstra {
public:
    // Reçoit un pointeur observateur sur la fourmilière 
    explicit AlgorithmeDijkstra(const Fourmiliere* fourmiliere);
    ~AlgorithmeDijkstra() = default;

    // Copie interdite — on ne duplique pas le pointeur de fourmilière
    AlgorithmeDijkstra(const AlgorithmeDijkstra&)            = delete;
    AlgorithmeDijkstra& operator=(const AlgorithmeDijkstra&) = delete;

    // Déplacement autorisé
    AlgorithmeDijkstra(AlgorithmeDijkstra&&)            = default;
    AlgorithmeDijkstra& operator=(AlgorithmeDijkstra&&) = default;

    // ---- Exécution ----
    // Calcule le plus court chemin Sv -> Sd et déplace toutes les fourmis
    ResultatDijkstra executer();

    // ---- Affichage ----
    void afficherResultat(const ResultatDijkstra& res) const;

private:
    // Algorithme pur — travaille sur les Salle* de la fourmilière
    ResultatDijkstra    _dijkstra() const;

    // Remonte le chemin depuis les prédécesseurs
    vector<Salle*>      _reconstruireChemin(
                            const map<string, string>& pred,
                            Salle* depart,
                            Salle* arrivee) const;

    // Déplace chaque Fourmi* jusqu'à la destination du chemin
    void                _deplacerFourmis(Salle* destination) const;

    // Pointeur observateur — la fourmilière reste propriétaire de ses données
    const Fourmiliere* _fourmiliere;
};