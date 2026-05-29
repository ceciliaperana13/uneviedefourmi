#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "Dijkstra.hpp"
#include "Visualiseur.hpp"
#include "fourmiliere.hpp"
#include "simulateur.hpp"

using namespace std;

class MenuPrincipal {
public:
    explicit MenuPrincipal(const string& dossierFourmilieres);
    void run();

private:

    static string nomAffichable(const string& chemin);
    vector<string> _fichiers;

    void        afficherMenu()       const;
    string choisirFourmiliere() const;

    void lancerDeepFirst()   const;
    void lancerDijkstra()    const;
    void lancerVisualiseur() const;

    static vector<string> listerFichiersTxt(const string& dossier);
};