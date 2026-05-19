#include "../include/lecteur_fichier_texte.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

// Lit le fichier ligne par ligne et dispatch chaque ligne vers le bon parseur
DonneesFourmiliere LecteurFichierTexte::lire(const string& chemin) {
    DonneesFourmiliere donnees;

    ifstream fichier(chemin);
    if (!fichier.is_open()) {
        cerr << "Impossible d'ouvrir : " << chemin << endl;
        return donnees;
    }

    // Collecte des lignes non vides pour le parsing en deux passes
    vector<string> lignes;
    string ligne;
    while (getline(fichier, ligne)) {
        ligne = trim(ligne);
        if (!ligne.empty()) lignes.push_back(ligne);
    }

    // Première passe : nombre de fourmis et déclarations de salles
    for (const auto& l : lignes) {
        if      (estLigneNbFourmis(l)) parseLigneNbFourmis(l, donnees);
        else if (!estLigneTunnel(l))   parseLigneSalle(l, donnees);
    }

    // Deuxième passe : tunnels (toutes les salles sont déjà connues)
    for (const auto& l : lignes) {
        if (estLigneTunnel(l)) parseLigneTunnel(l, donnees);
    }

    return donnees;
}

// Extrait le nombre de fourmis depuis une ligne de la forme "f=50"
void LecteurFichierTexte::parseLigneNbFourmis(const string& ligne, DonneesFourmiliere& donnees) {
    donnees.nbFourmis = stoi(ligne.substr(2));
}

// Enregistre une salle avec son nom et sa capacité ("S1" ou "S1 { 8 }")
// Sv et Sd sont ignorés : ils sont créés en dur par Fourmiliere
void LecteurFichierTexte::parseLigneSalle(const string& ligne, DonneesFourmiliere& donnees) {
    istringstream iss(ligne);
    string nom;
    iss >> nom;

    if (nom == "Sv" || nom == "Sd") return;

    // Capacité explicite entre accolades, sinon 1 par défaut
    int capacite = (ligne.find('{') != string::npos)
        ? extraireCapacite(ligne)
        : 1;

    donnees.salles.emplace_back(nom, capacite);
}

// Enregistre un tunnel entre deux salles depuis une ligne "SA - SB"
void LecteurFichierTexte::parseLigneTunnel(const string& ligne, DonneesFourmiliere& donnees) {
    istringstream iss(ligne);
    string nomA, tiret, nomB;
    iss >> nomA >> tiret >> nomB;
    donnees.tunnels.emplace_back(nomA, nomB);
}

// Détecte une ligne de type "f=N"
bool LecteurFichierTexte::estLigneNbFourmis(const string& ligne) {
    return ligne.size() >= 2 && ligne.substr(0, 2) == "f=";
}

// Détecte une ligne de tunnel via la présence de " - "
bool LecteurFichierTexte::estLigneTunnel(const string& ligne) {
    return ligne.find(" - ") != string::npos;
}

// Extrait la valeur entière entre accolades dans "S1 { 8 }"
int LecteurFichierTexte::extraireCapacite(const string& ligne) {
    size_t debut = ligne.find('{');
    size_t fin   = ligne.find('}');
    if (debut == string::npos || fin == string::npos) return 1;
    return stoi(trim(ligne.substr(debut + 1, fin - debut - 1)));
}

// Supprime les espaces et caractères invisibles en début et fin de chaîne
string LecteurFichierTexte::trim(const string& s) {
    size_t debut = s.find_first_not_of(" \t\r\n");
    if (debut == string::npos) return "";
    size_t fin = s.find_last_not_of(" \t\r\n");
    return s.substr(debut, fin - debut + 1);
}