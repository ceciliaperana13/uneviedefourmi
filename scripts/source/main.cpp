#include "../include/main.hpp"
#include "../include/fourmiliere.hpp"
#include "../include/Dijkstra.hpp"
#include "../include/Visualiseur.hpp"
#include "../include/simulateur.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <limits>

using namespace std;
using namespace std::chrono;

const string BASE = "../../fourmilieres/";

// ============================================================
//  Liste unique des fourmilières
// ============================================================
static const vector<string> LISTE_FOURMILIERES = {
    BASE + "fourmiliere_zero.txt",
    BASE + "fourmiliere_un.txt",
    BASE + "fourmiliere_deux.txt",
    BASE + "fourmiliere_trois.txt",
    BASE + "fourmiliere_quatre.txt",
    BASE + "fourmiliere_cinq.txt",
    BASE + "salle_d_at-ant.txt",
    BASE + "La_hormiguera_de_la_muerte.txt"
};

// ============================================================
//  Sélection de la fourmilière
// ============================================================
static string choisirFourmiliere() {
    cout << "\nChoisissez une fourmilière :\n";
    for (int i = 0; i < (int)LISTE_FOURMILIERES.size(); i++) {
        cout << "  " << i+1 << " — " << LISTE_FOURMILIERES[i] << "\n";
    }
    cout << "Votre choix : ";

    int choix = 0;
    cin >> choix;

    if (choix < 1 || choix > (int)LISTE_FOURMILIERES.size()) {
        cout << "[ERREUR] Choix invalide.\n";
        return "";
    }

    return LISTE_FOURMILIERES[choix - 1];
}

// ============================================================
//  Deep First
// ============================================================
static void lancerDeepFirst() {
    string fichier = choisirFourmiliere();
    if (fichier.empty()) return;

    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(fichier)) {
        cerr << "[ERREUR] Impossible de charger " << fichier << endl;
        return;
    }

    fm.afficher();

    Simulateur sim(fm);
    sim.simuler();
}

// ============================================================
//  Dijkstra
// ============================================================
static void lancerDijkstra() {
    string fichier = choisirFourmiliere();
    if (fichier.empty()) return;

    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(fichier)) {
        cerr << "[ERREUR] Impossible de charger " << fichier << endl;
        return;
    }

    fm.afficher();

    AlgorithmeDijkstra algo(&fm);
    ResultatDijkstra res = algo.executer();

    algo.afficherResultat(res);
}

// ============================================================
//  Visualiseur SFML
// ============================================================
static void lancerVisualiseur() {
    string fichier = choisirFourmiliere();
    if (fichier.empty()) return;

    // Charger la fourmilière
    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(fichier)) {
        cerr << "[ERREUR] Impossible de charger " << fichier << endl;
        return;
    }

    // Lancer Deep‑First pour obtenir les étapes
    Simulateur sim(fm);
    sim.simuler();

    // Visualiseur
    Visualiseur vis(1280, 800);

  

    // Lancer l'affichage
    vis.run(LISTE_FOURMILIERES);
}

// ============================================================
//  Menu principal
// ============================================================
static void afficherMenu() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║        SIMULATION DE FOURMILIERES            ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  1  —  Deep First + Round Robin Nelson       ║\n";
    cout << "║  2  —  Dijkstra                              ║\n";
    cout << "║  3  —  Visualiseur graphique                 ║\n";
    cout << "║  0  —  Quitter                               ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "Votre choix : ";
}

// ============================================================
//  Main
// ============================================================
int main() {
    int choix = -1;

    while (choix != 0) {
        afficherMenu();

        if (!(cin >> choix)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "[ERREUR] Entree invalide.\n";
            continue;
        }

        switch (choix) {
            case 1: lancerDeepFirst();   break;
            case 2: lancerDijkstra();    break;
            case 3: lancerVisualiseur(); break;
            case 0: cout << "Au revoir !\n"; break;
            default: cout << "[ERREUR] Choix invalide.\n"; break;
        }
    }

    return 0;
}
