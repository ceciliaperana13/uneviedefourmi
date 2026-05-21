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

class MenuPrincipal {
public:
    explicit MenuPrincipal(const std::string& dossierFourmilieres);
    void run();

private:

    static std::string nomAffichable(const std::string& chemin);
    std::vector<std::string> _fichiers;

    void        afficherMenu()       const;
    std::string choisirFourmiliere() const;

    void lancerDeepFirst()   const;
    void lancerDijkstra()    const;
    void lancerVisualiseur() const;

    static std::vector<std::string> listerFichiersTxt(const std::string& dossier);
};