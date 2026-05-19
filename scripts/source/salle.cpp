#include "../include/salle.hpp"

class Salle {
public:
    static constexpr int CAPACITE_ILLIMITEE = -1;

    Salle(const std::string& nom, int capacite = 1);

    void ajouterVoisin(Salle* voisin);
    bool peutAccueillir() const;
    bool estConnecteA(const Salle* autre) const;

    // On doit pouvoir reserver les emplacements d'une salle pour l'étape suivante 
    void reserver();
    void liberer();
    void entrer();
    void sortir();

    bool estVestibule() const;
    bool estDortoir() const;

    const std::string& getNom() const;
    int getCapacite() const;
    int getOccupants() const;
    const std::vector<Salle*>& getVoisins() const;

private:
    std::string nom;
    int capacite;
    int occupants;   // Fourmis à l'interieur
    int reserves;    // Emplacements réservés pour l'étape suivante 
    std::vector<Salle*> voisins;
};