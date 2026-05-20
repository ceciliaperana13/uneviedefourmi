#include "../include/main.hpp"

// Charge une fourmilière depuis un fichier et lance la simulation
void traiterFourmiliere(const string& chemin) {
    Fourmiliere fourmiliere;

    // Tente de charger la fourmiliere depuis le path d'un .txt
    if (!fourmiliere.chargerDepuisFichier(chemin)) {
        cerr << "Echec du chargement : " << chemin << endl;
        return;
    }

    cout << "\n========== " << chemin << " ==========" << endl;
    // Affiche une représentation schématique de la fourmilière
    fourmiliere.afficher();
    cout << endl;

    // Résoud la fourmilière et affiche les étapes 
    Simulateur simulateur(fourmiliere);
    simulateur.simuler();
}

void traiterToutesLesFourmilieresParDeepFirst() {

    cout << "===================++++++++++++++++++++++++++++++++++++++++++======================" << endl;
    cout << "=================== PAR DEEP FIRST + ALGO ROUND ROBIN NELSON ======================" << endl;
    cout << "===================++++++++++++++++++++++++++++++++++++++++++======================" << endl;

    traiterFourmiliere("../../fourmilieres/fourmiliere_zero.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_un.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_deux.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_trois.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_quatre.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_cinq.txt");

    // traiterFourmiliere("../../fourmilieres/fourmiliere_3D.txt"); CASSE MON ALGORITHME POUR L'INSTANT 
    traiterFourmiliere("../../fourmilieres/salle_d_at-ant.txt");
    traiterFourmiliere("../../fourmilieres/La_hormiguera_de_la_muerte.txt");
}

int main() {

    traiterToutesLesFourmilieresParDeepFirst(); 
    return 0;
}