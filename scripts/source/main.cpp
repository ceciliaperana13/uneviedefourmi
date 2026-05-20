#include "../include/main.hpp"

// Charge une fourmilière depuis un fichier et lance la simulation
void traiterFourmiliere(const string& chemin) {
    Fourmiliere fourmiliere;

    if (!fourmiliere.chargerDepuisFichier(chemin)) {
        cerr << "Echec du chargement : " << chemin << endl;
        return;
    }

    cout << "\n========== " << chemin << " ==========" << endl;
    fourmiliere.afficher();
    cout << endl;

    Simulateur simulateur(fourmiliere);
    simulateur.simuler();
}

int main() {
    traiterFourmiliere("../../fourmilieres/fourmiliere_zero.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_un.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_deux.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_trois.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_quatre.txt");
    traiterFourmiliere("../../fourmilieres/fourmiliere_cinq.txt");

    // traiterFourmiliere("../../fourmilieres/fourmiliere_3D.txt");
    traiterFourmiliere("../../fourmilieres/salle_d_at-ant.txt");
    traiterFourmiliere("../../fourmilieres/La_hormiguera_de_la_muerte.txt");

    return 0;
}