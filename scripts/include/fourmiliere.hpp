#pragma once
#include <map>
#include <string>
#include <vector>
#include "fourmi.hpp"
#include "lecteur_fichier_texte.hpp"
#include "salle.hpp"

using namespace std;

class Fourmiliere {
public:
    Fourmiliere();
    ~Fourmiliere();

    // Délègue la lecture au LecteurFichierTexte puis construit le graphe
    bool chargerDepuisFichier(const string& chemin);
    void afficher() const;

    int    getNbFourmis() const;
    Salle* getSalle    (const string& nom) const;
    Salle* getVestibule() const;
    Salle* getDortoir  () const;

    const map<string, Salle*>& getSalles()  const;
    const vector<Fourmi*>&          getFourmis() const;

private:
    int                           nbFourmis;
    map<string, Salle*> salles;
    vector<Fourmi*>          fourmis;

    // Construction du graphe à partir des données brutes parsées
    void construireDepuisDonnees(const DonneesFourmiliere& donnees);
    void ajouterSalle  (const string& nom, int capacite);
    void ajouterTunnel (const string& nomA, const string& nomB);

    void   initialiserSallesSpeciales();
    void   initialiserFourmis();
    Salle* getOuCreerSalle(const string& nom, int capacite = 1);
};