#pragma once
#include <string>

class Salle;

class Fourmi {
public:
    Fourmi(int id, Salle* salleDepart);

    // Pose une réservation sur la destination sans bouger physiquement
    // Retourne false si la destination ne peut pas accueillir
    bool planifierDeplacement(Salle* destination);

    // Effectue le déplacement physique après validation de la réservation
    void commitDeplacement();

    // Annule la réservation posée par planifierDeplacement
    void annulerDeplacement();

    bool estAuDortoir() const;

    // Formate le mouvement pour l'affichage : "fN - origine - destination"
    // À appeler après commitDeplacement
    std::string formatDeplacement() const;

    int    getId()             const;
    Salle* getSalleActuelle()  const;
    Salle* getDestination()    const;

private:
    int    id;
    Salle* salleActuelle;
    Salle* prochaineSalle;  // destination réservée, nullptr si pas de mouvement prévu
};