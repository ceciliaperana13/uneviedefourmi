#include "../include/algo_deep_first.hpp"
#include <algorithm>
#include <queue>

// Point d'entrée public.
// Calcule d'abord les distances BFS depuis Sd, puis lance le DFS filtré,
// puis trie les résultats par longueur croissante.
std::vector<std::vector<Salle*>> AlgoDeepFirst::trouverTousLesChemins(const Fourmiliere& fourmiliere) {
    std::vector<std::vector<Salle*>> resultats;
    std::vector<Salle*>              cheminCourant;

    Salle* depart      = fourmiliere.getVestibule();
    Salle* destination = fourmiliere.getDortoir();

    std::map<std::string, int> distancesVersSd = calculerDistancesVersSd(fourmiliere);

    cheminCourant.push_back(depart);
    explorer(depart, depart, destination, cheminCourant, resultats, distancesVersSd);

    // Tri par longueur : les fourmis assignées aux chemins courts avancent plus vite,
    // ce qui maximise le débit global du pipeline
    std::sort(resultats.begin(), resultats.end(),
        [](const std::vector<Salle*>& a, const std::vector<Salle*>& b) {
            return a.size() < b.size();
        });

    return resultats;
}

// BFS depuis Sd sur le graphe non orienté.
// La distance obtenue est le nombre minimal de tunnels pour atteindre Sd.
std::map<std::string, int> AlgoDeepFirst::calculerDistancesVersSd(const Fourmiliere& fourmiliere) {
    std::map<std::string, int> distances;
    const std::map<std::string, Salle*>& salles = fourmiliere.getSalles();

    for (const auto& kv : salles)
        distances[kv.first] = INT_MAX;

    Salle* sd = fourmiliere.getDortoir();
    distances[sd->getNom()] = 0;

    std::queue<Salle*> file;
    file.push(sd);

    while (!file.empty()) {
        Salle* courante = file.front();
        file.pop();

        for (Salle* voisin : courante->getVoisins()) {
            if (distances[voisin->getNom()] == INT_MAX) {
                distances[voisin->getNom()] = distances[courante->getNom()] + 1;
                file.push(voisin);
            }
        }
    }

    return distances;
}

// Principe : on descend aussi profond que possible avant de remonter (backtrack).
//
// Filtre de distance :
//   - Depuis Sv (depart) : tous les voisins sont autorisés.
//     Sv et certains de ses voisins (ex: S1) peuvent être à égale distance de Sd.
//     Bloquer ces voisins priverait le DFS de branches entières de chemins utiles.
//   - Depuis toute autre salle : on rejette les voisins à distance >= distance courante.
//     Cela empêche les mouvements latéraux (ex: S3↔S5, même distance) qui causent
//     des deadlocks circulaires quand des fourmis se croisent dans des salles à
//     capacité limitée.
void AlgoDeepFirst::explorer(
    Salle*                            courante,
    Salle*                            depart,
    Salle*                            destination,
    std::vector<Salle*>&              cheminCourant,
    std::vector<std::vector<Salle*>>& resultats,
    const std::map<std::string, int>& distancesVersSd)
{
    if (courante == destination) {
        resultats.push_back(cheminCourant);
        return;
    }

    int distanceCourante = distancesVersSd.at(courante->getNom());

    for (Salle* voisin : courante->getVoisins()) {
        bool dejaVisite = std::find(cheminCourant.begin(), cheminCourant.end(), voisin)
                          != cheminCourant.end();
        if (dejaVisite) continue;

        // Depuis Sv, on ne filtre pas : on veut explorer toutes les branches de départ.
        // Depuis les autres salles, on exige une progression stricte vers Sd.
        if (courante != depart) {
            int distanceVoisin = distancesVersSd.at(voisin->getNom());
            if (distanceVoisin >= distanceCourante) continue;
        }

        cheminCourant.push_back(voisin);
        explorer(voisin, depart, destination, cheminCourant, resultats, distancesVersSd);
        cheminCourant.pop_back();
    }
}