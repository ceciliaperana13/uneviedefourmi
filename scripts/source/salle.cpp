#include "../include/salle.hpp"
#include <algorithm>

// Initialise la salle avec son nom, sa capacité, et tous les compteurs à zéro
Salle::Salle(const string& nom, int capacite)
    : nom(nom), capacite(capacite), occupants(0), reserves(0), partants(0) {}

// Ajoute un voisin uniquement s'il n'est pas déjà enregistré (évite les doublons)
void Salle::ajouterVoisin(Salle* voisin) {
    if (!estConnecteA(voisin))
        voisins.push_back(voisin);
}

// Une salle peut accueillir si : les occupants qui restent + les arrivées prévues < capacité
// On soustrait les partants car leurs places sont libérables dans la même étape
bool Salle::peutAccueillir() const {
    if (capacite == CAPACITE_ILLIMITEE) return true;
    return (occupants - partants + reserves) < capacite;
}

bool Salle::estConnecteA(const Salle* autre) const {
    return find(voisins.begin(), voisins.end(), autre) != voisins.end();
}

// --- Phase de planification ---

// Une fourmi a réservé une place pour entrer dans cette salle
void Salle::reserver() { reserves++; }

// Annule une réservation d'entrée (si l'algorithme change d'avis)
void Salle::liberer() { if (reserves > 0) reserves--; }

// Une fourmi a prévu de quitter cette salle — sa place est libérable dès maintenant
void Salle::programmerDepart() { partants++; }

// Annule un départ prévu
void Salle::annulerDepart() { if (partants > 0) partants--; }

// --- Phase de commit ---

// La fourmi entre physiquement : consomme sa réservation
void Salle::entrer() {
    occupants++;
    if (reserves > 0) reserves--;
}

// La fourmi part physiquement : consomme son départ prévu
void Salle::sortir() {
    if (occupants > 0) occupants--;
    if (partants > 0) partants--;
}

bool Salle::estVestibule() const { return nom == "Sv"; }
bool Salle::estDortoir()   const { return nom == "Sd"; }

const string&         Salle::getNom()       const { return nom; }
int                   Salle::getCapacite()  const { return capacite; }
int                   Salle::getOccupants() const { return occupants; }
const vector<Salle*>& Salle::getVoisins()   const { return voisins; }
