#include "../include/algo_deep_first.hpp"

// Point d'entrée public.
// Calcule d'abord les distances BFS depuis Sd, puis lance le DFS filtré,
// puis trie les résultats par longueur croissante.
vector<vector<Salle*>> AlgoDeepFirst::trouverTousLesChemins(const Fourmiliere& fourmiliere) {
    vector<vector<Salle*>> resultats;
    vector<Salle*>              cheminCourant;

    Salle* depart      = fourmiliere.getVestibule();
    Salle* destination = fourmiliere.getDortoir();

    map<string, int> distancesVersSd = calculerDistancesVersSd(fourmiliere);

    cheminCourant.push_back(depart);
    explorer(depart, depart, destination, cheminCourant, resultats, distancesVersSd);

    // Tri par longueur : les fourmis assignées aux chemins courts avancent plus vite,
    // ce qui maximise le débit global du pipeline
    sort(resultats.begin(), resultats.end(),
        [](const vector<Salle*>& a, const vector<Salle*>& b) {
            return a.size() < b.size();
        });

    return resultats;
}

// BFS depuis Sd sur le graphe non orienté.
// La distance obtenue est le nombre minimal de tunnels pour atteindre Sd.
map<string, int> AlgoDeepFirst::calculerDistancesVersSd(const Fourmiliere& fourmiliere) {
    map<string, int> distances;
    const map<string, Salle*>& salles = fourmiliere.getSalles();

    for (const auto& kv : salles)
        distances[kv.first] = INT_MAX;

    Salle* sd = fourmiliere.getDortoir();
    distances[sd->getNom()] = 0;

    queue<Salle*> file;
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
    vector<Salle*>&              cheminCourant,
    vector<vector<Salle*>>& resultats,
    const map<string, int>& distancesVersSd)
{
    if (courante == destination) {
        resultats.push_back(cheminCourant);
        return;
    }

    int distanceCourante = distancesVersSd.at(courante->getNom());

    for (Salle* voisin : courante->getVoisins()) {
        bool dejaVisite = find(cheminCourant.begin(), cheminCourant.end(), voisin)
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