#pragma once
#include <string>
#include <vector>

using namespace std;

class Salle {
public:
    static constexpr int CAPACITE_ILLIMITEE = -1;

    Salle(const string& nom, int capacite = 1);

    void ajouterVoisin(Salle* voisin);
    bool peutAccueillir() const;
    bool estConnecteA(const Salle* autre) const;

    // Pouvoir réserver des espaces pour l'étape suivante! 
    void reserver();
    void liberer();
    void entrer();
    void sortir();

    bool estVestibule() const;
    bool estDortoir() const;

    const string& getNom() const;
    int getCapacite() const;
    int getOccupants() const;
    const vector<Salle*>& getVoisins() const;

private:
    string nom;
    int capacite;
    int occupants;   // Fourmis à l'interieur à une étape N
    int reserves;    // Emplacements réservés pour l'étape N + 1 
    vector<Salle*> voisins;
};