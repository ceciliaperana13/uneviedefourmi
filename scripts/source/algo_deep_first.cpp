#include "../include/algo_deep_first.hpp"
#include <algorithm>

// Point d'entrée public.
// Initialise le chemin courant avec Sv, lance le DFS, puis trie les résultats
// par longueur croissante pour que le Simulateur priorise les chemins courts.
std::vector<std::vector<Salle*>> AlgoDeepFirst::trouverTousLesChemins(const Fourmiliere& fourmiliere) {
    std::vector<std::vector<Salle*>> resultats;
    std::vector<Salle*> cheminCourant;

    Salle* depart      = fourmiliere.getVestibule();
    Salle* destination = fourmiliere.getDortoir();

    // On part du vestibule : il est toujours le premier noeud de chaque chemin
    cheminCourant.push_back(depart);
    explorer(depart, destination, cheminCourant, resultats);

    // Tri par longueur : les fourmis assignées aux chemins courts avancent plus vite,
    // ce qui maximise le débit global du pipeline
    std::sort(resultats.begin(), resultats.end(),
        [](const std::vector<Salle*>& a, const std::vector<Salle*>& b) {
            return a.size() < b.size();
        });

    return resultats;
}

// Principe : on descend aussi profond que possible dans le graphe avant de
// remonter et d'essayer un autre voisin.
// cheminCourant est passé par référence et modifié à chaque appel :
// resultats accumule les chemins complets trouvés au fil de la récursion.
void AlgoDeepFirst::explorer(
    Salle*                            courante,
    Salle*                            destination,
    std::vector<Salle*>&              cheminCourant,
    std::vector<std::vector<Salle*>>& resultats)
{
    // Cas de base : on a atteint le dortoir donc le chemin courant est complet, on l'enregistre
    if (courante == destination) {
        resultats.push_back(cheminCourant);
        return;
    }

    // On tente d'explorer chaque voisin de la salle courante
    for (Salle* voisin : courante->getVoisins()) {

        // On refuse de revisiter une salle déjà présente dans le chemin courant.
        bool dejaVisite = std::find(cheminCourant.begin(), cheminCourant.end(), voisin)
                          != cheminCourant.end();
        if (dejaVisite) continue;

        // Descente : on ajoute ce voisin au chemin et on explore depuis lui
        cheminCourant.push_back(voisin);
        explorer(voisin, destination, cheminCourant, resultats);

        // Backtrack : on retire ce voisin pour explorer les autres branches
        // Sans cette ligne, les prochains chemins contiendraient les noeuds
        // des branches déjà explorées
        cheminCourant.pop_back();
    }
}