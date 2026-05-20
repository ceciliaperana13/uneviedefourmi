#pragma once
#include <vector>
#include "fourmiliere.hpp"
#include "salle.hpp"

class AlgoDeepFirst {
public:
    // Retourne tous les chemins simples Sv - Sd, triés du plus court au plus long
    static std::vector<std::vector<Salle*>> trouverTousLesChemins(const Fourmiliere& fourmiliere);

private:
    // Exploration récursive deep-first
    static void explorer(
        Salle*                            courante,
        Salle*                            destination,
        std::vector<Salle*>&              cheminCourant,
        std::vector<std::vector<Salle*>>& resultats
    );
};