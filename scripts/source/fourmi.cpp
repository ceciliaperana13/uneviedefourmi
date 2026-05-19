#include "../include/fourmi.hpp"
#include "../include/salle.hpp"

Fourmi::Fourmi(int id, Salle* salleDepart)
    : id(id), salleActuelle(salleDepart) {
    salleDepart->entrer();
}

bool Fourmi::seDeplacer(Salle* destination) {
    if (!destination->peutAccueillir()) return false;
    salleActuelle->sortir();
    destination->entrer();
    salleActuelle = destination;
    return true;
}

bool Fourmi::estAuDortoir() const {
    return salleActuelle->estDortoir();
}

string Fourmi::formatDeplacement(const string& nomOrigine) const {
    return "f" + to_string(id)
        + " - " + nomOrigine
        + " - " + salleActuelle->getNom();
}

int    Fourmi::getId()             const { return id; }
Salle* Fourmi::getSalleActuelle()  const { return salleActuelle; }