#pragma once
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class Salle {
public:
    static constexpr int CAPACITE_ILLIMITEE = -1;

    Salle(const string& nom, int capacite = 1);

    void ajouterVoisin(Salle* voisin);
    bool peutAccueillir() const;
    bool estConnecteA(const Salle* autre) const;

    // Phase de planification
    void reserver();          // une fourmi va entrer
    void liberer();           // annule une réservation d'entrée
    void programmerDepart();  // une fourmi va partir
    void annulerDepart();     // annule un départ prévu

    // Phase de commit
    void entrer();  // la fourmi entre physiquement (consomme une réservation)
    void sortir();  // la fourmi part physiquement (consomme un départ prévu)

    bool estVestibule() const;
    bool estDortoir() const;

    const string& getNom() const;
    int getCapacite() const;
    int getOccupants() const;
    const vector<Salle*>& getVoisins() const;

private:
    string nom;
    int capacite;
    int occupants;  // fourmis physiquement présentes
    int reserves;   // fourmis qui vont entrer (planifié)
    int partants;   // fourmis qui vont partir (planifié)
    vector<Salle*> voisins;
};