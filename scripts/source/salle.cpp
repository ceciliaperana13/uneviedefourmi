#include "../include/salle.hpp"
#include <algorithm>

// Initialise la salle avec son nom, sa capacité, et zéro occupant/réservation
Salle::Salle(const string& nom, int capacite)
    : nom(nom), capacite(capacite), occupants(0), reserves(0) {}

// Ajoute un voisin uniquement s'il n'est pas déjà enregistré (évite les doublons)
void Salle::ajouterVoisin(Salle* voisin) {
    if (!estConnecteA(voisin))
        voisins.push_back(voisin);
}

// Un slot est libre si ni occupé ni réservé pour l'étape en cours
bool Salle::peutAccueillir() const {
    if (capacite == CAPACITE_ILLIMITEE) return true;
    return (occupants + reserves) < capacite;
}

bool Salle::estConnecteA(const Salle* autre) const {
    return find(voisins.begin(), voisins.end(), autre) != voisins.end();
}

// --- Gestion des réservations (phase de planification d'une étape) ---

// Réserve un slot : bloque la place avant que la fourmi ne soit physiquement entrée
void Salle::reserver() { reserves++; }

// Annule une réservation (si le déplacement est finalement impossible)
void Salle::liberer() { if (reserves > 0) reserves--; }

// --- Gestion des occupants (phase de commit d'une étape) ---

// Marque l'entrée physique d'une fourmi et consomme sa réservation
void Salle::entrer() {
    occupants++;
    if (reserves > 0) reserves--;
}

// Marque le départ physique d'une fourmi
void Salle::sortir() { if (occupants > 0) occupants--; }

bool Salle::estVestibule() const { return nom == "Sv"; }
bool Salle::estDortoir()   const { return nom == "Sd"; }

const string&          Salle::getNom()       const { return nom; }
int                    Salle::getCapacite()  const { return capacite; }
int                    Salle::getOccupants() const { return occupants; }
const vector<Salle*>&  Salle::getVoisins()   const { return voisins; }