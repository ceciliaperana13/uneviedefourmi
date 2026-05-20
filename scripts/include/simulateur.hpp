#pragma once
#include <map>
#include <string>
#include <vector>
#include "algo_deep_first.hpp"
#include "fourmi.hpp"
#include "fourmiliere.hpp"
#include "salle.hpp"

using namespace std;

class Simulateur {
public:
    Simulateur(const Fourmiliere& fourmiliere);

    // Point d'entrée : lance la simulation complète et affiche les étapes
    void simuler();

private:
    const Fourmiliere& fourmiliere;
    vector<vector<Salle*>> chemins;
    map<Fourmi*, vector<Salle*>> assignation; // fourmi : son chemin
    map<Fourmi*, int> positionSurChemin; // fourmi : son index actuel
    map<Fourmi*, bool> aPlanifie; // fourmi : a bougé ce tour
    int numeroEtape;

    void assignerFourmisALeursChemins();
    void executerUneEtape();
    bool toutesAuDortoir() const;
};