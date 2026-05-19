#pragma once
#include <string>
#include <utility>
#include <vector>

using namespace std;

// Données brutes extraites du fichier texte, avant construction du graphe
struct DonneesFourmiliere {
    int nbFourmis = 0;
    vector<pair<string, int>>                salles;  // (nom, capacite)
    vector<pair<string, string>>        tunnels; // (nomA, nomB)
};

// Responsabilité unique : lire un fichier texte et en extraire les données brutes
class LecteurFichierTexte {
public:
    // Point d'entrée : retourne les données parsées ou une struct vide si échec
    static DonneesFourmiliere lire(const string& chemin);

private:
    static void parseLigneNbFourmis(const string& ligne, DonneesFourmiliere& donnees);
    static void parseLigneSalle    (const string& ligne, DonneesFourmiliere& donnees);
    static void parseLigneTunnel   (const string& ligne, DonneesFourmiliere& donnees);

    static bool estLigneNbFourmis(const string& ligne);
    static bool estLigneTunnel   (const string& ligne);

    static int         extraireCapacite(const string& ligne);
    static string trim            (const string& s);
};