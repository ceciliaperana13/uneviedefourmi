#include "../include/text_file_reader.hpp"

string lireUneFourmiliere(string pathDuFichierTexte) { 

    // Création de la string qui va contenir le texte du fichier 
    string fourmiliereString;

    // Lecture du fichier
    ifstream fourmiliereReadFile(pathDuFichierTexte);

    cout << "==== CONTENU DU FICHIER TEXTE " << pathDuFichierTexte << " ====" << endl;
    // Boucle while pour lire le fichier ligne par ligne
    while (getline (fourmiliereReadFile, fourmiliereString)) {
        // Output les lignes
        cout << fourmiliereString << endl;
    }

    // Fermer le fichier 
    fourmiliereReadFile.close();

    return fourmiliereString;
}

int main() {

    string fourmiliereZeroString = lireUneFourmiliere("../../fourmilieres/fourmiliere_zero.txt");
    string fourmiliereUnString = lireUneFourmiliere("../../fourmilieres/fourmiliere_un.txt");
    string fourmiliereDeuxString = lireUneFourmiliere("../../fourmilieres/fourmiliere_deux.txt");
    string fourmiliereTroisString = lireUneFourmiliere("../../fourmilieres/fourmiliere_trois.txt");
    string fourmiliereQuatreString = lireUneFourmiliere("../../fourmilieres/fourmiliere_quatre.txt");
    string fourmiliereCinqString = lireUneFourmiliere("../../fourmilieres/fourmiliere_cinq.txt");

    return 0;
    }