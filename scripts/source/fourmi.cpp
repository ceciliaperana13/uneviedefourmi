#include "../include/fourmi.hpp"

// Toutes les fourmis démarrent dans le vestibule, sans destination prévue
Fourmi::Fourmi(int id, Salle* salleDepart)
    : id(id), salleActuelle(salleDepart), prochaineSalle(nullptr) {
    salleDepart->entrer();
}

// Phase 1 d'une étape : réserve la destination et programme le départ de la salle actuelle
// Retourne false si la destination ne peut pas accueillir
bool Fourmi::planifierDeplacement(Salle* destination) {
    if (!destination->peutAccueillir()) return false;
    destination->reserver();
    salleActuelle->programmerDepart(); // libère la place pour les autres fourmis dès cette étape
    prochaineSalle = destination;
    return true;
}

// Phase 2 : effectue le déplacement physique
// sortir() consomme le départ prévu, entrer() consomme la réservation
void Fourmi::commitDeplacement() {
    if (prochaineSalle == nullptr) return;
    salleActuelle->sortir();
    prochaineSalle->entrer();
    salleActuelle  = prochaineSalle;
    prochaineSalle = nullptr;
}

// Annule la planification : libère la réservation et annule le départ prévu
void Fourmi::annulerDeplacement() {
    if (prochaineSalle == nullptr) return;
    prochaineSalle->liberer();
    salleActuelle->annulerDepart();
    prochaineSalle = nullptr;
}

bool Fourmi::estAuDortoir() const {
    return salleActuelle->estDortoir();
}

// À appeler AVANT commitDeplacement — prochaineSalle sera nullptr après
string Fourmi::formatDeplacement() const {
    return "f" + to_string(id)
        + " - " + salleActuelle->getNom()
        + " - " + prochaineSalle->getNom();
}

int    Fourmi::getId()            const { return id; }
Salle* Fourmi::getSalleActuelle() const { return salleActuelle; }
Salle* Fourmi::getDestination()   const { return prochaineSalle; }