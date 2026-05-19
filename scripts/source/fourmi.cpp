#include "../include/fourmi.hpp"
#include "../include/salle.hpp"

// Toutes les fourmis démarrent dans le vestibule, sans destination prévue
Fourmi::Fourmi(int id, Salle* salleDepart)
    : id(id), salleActuelle(salleDepart), prochaineSalle(nullptr) {
    salleDepart->entrer();
}

// Phase 1 d'une étape : réserve la destination sans bouger
// Retourne false si la salle est pleine (réservations + occupants >= capacité)
bool Fourmi::planifierDeplacement(Salle* destination) {
    if (!destination->peutAccueillir()) return false;
    destination->reserver();
    prochaineSalle = destination;
    return true;
}

// Phase 2 d'une étape : effectue le déplacement physique
// À appeler uniquement si planifierDeplacement a retourné true
void Fourmi::commitDeplacement() {
    if (prochaineSalle == nullptr) return;
    salleActuelle->sortir();
    prochaineSalle->entrer(); // entrer() consomme la réservation posée plus tôt
    salleActuelle  = prochaineSalle;
    prochaineSalle = nullptr;
}

// Annule une réservation posée (si l'algorithme décide de ne pas déplacer)
void Fourmi::annulerDeplacement() {
    if (prochaineSalle == nullptr) return;
    prochaineSalle->liberer();
    prochaineSalle = nullptr;
}

bool Fourmi::estAuDortoir() const {
    return salleActuelle->estDortoir();
}

// Formate le mouvement "fN - origine - destination" pour l'affichage de l'étape
std::string Fourmi::formatDeplacement() const {
    return "f" + std::to_string(id)
        + " - " + salleActuelle->getNom()
        + " - " + prochaineSalle->getNom();
}

int    Fourmi::getId()            const { return id; }
Salle* Fourmi::getSalleActuelle() const { return salleActuelle; }
Salle* Fourmi::getDestination()   const { return prochaineSalle; }