#pragma once

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>

#include "fourmiliere.hpp"
#include "salle.hpp"

class AlgoDeepFirst {
public:
    // Retourne tous les chemins simples Sv -> Sd, triés du plus court au plus long.
    // Seuls les chemins strictement monotones vers Sd sont conservés (hors Sv),
    // ce qui élimine les croisements et deadlocks en simulation.
    static vector<vector<Salle*>> trouverTousLesChemins(const Fourmiliere& fourmiliere);

private:
    // BFS depuis Sd pour calculer la distance minimale de chaque salle vers Sd
    static map<string, int> calculerDistancesVersSd(const Fourmiliere& fourmiliere);

    // Exploration récursive deep-first.
    // depart est passé pour désactiver le filtre de distance uniquement sur Sv :
    // depuis Sv, tous les voisins sont explorés pour maximiser les chemins disponibles.
    // Depuis toute autre salle, seuls les voisins strictement plus proches de Sd sont explorés.
    static void explorer(
        Salle*                            courante,
        Salle*                            depart,
        Salle*                            destination,
        vector<Salle*>&              cheminCourant,
        vector<vector<Salle*>>& resultats,
        const map<string, int>& distancesVersSd
    );
};