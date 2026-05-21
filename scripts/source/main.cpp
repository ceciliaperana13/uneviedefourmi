#include "../include/main.hpp"
#include "../include/fourmiliere.hpp"
#include "../include/Dijkstra.hpp"
#include "../include/Visualiseur.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <limits>

using namespace std;
using namespace std::chrono;

const string BASE = "../../fourmilieres/";

// ============================================================
//  Utilitaires d'affichage
// ============================================================
static void separateur(const string& titre) {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  " << titre;
    for (int i = titre.size(); i < 44; i++) cout << ' ';
    cout << "║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
}

// ============================================================
//  ALGORITHME 1 — Deep First + Round Robin Nelson
// ============================================================
static void traiterDeepFirst(const string& chemin) {
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

static void lancerDeepFirst() {
    cout << "\n";
    cout << "============================= ++++++++++++++++++++++++++++++++++++++++ ================================\n";
    cout << "============================= PAR DEEP FIRST + ALGO ROUND ROBIN NELSON ================================\n";
    cout << "============================= ++++++++++++++++++++++++++++++++++++++++ ================================\n";

    traiterDeepFirst(BASE + "fourmiliere_zero.txt");
    traiterDeepFirst(BASE + "fourmiliere_un.txt");
    traiterDeepFirst(BASE + "fourmiliere_deux.txt");
    traiterDeepFirst(BASE + "fourmiliere_trois.txt");
    traiterDeepFirst(BASE + "fourmiliere_quatre.txt");
    traiterDeepFirst(BASE + "fourmiliere_cinq.txt");
    traiterDeepFirst(BASE + "fourmiliere_3D.txt");
    traiterDeepFirst(BASE + "salle_d_at-ant.txt");
    traiterDeepFirst(BASE + "La_hormiguera_de_la_muerte.txt");

    cout << "\n";
    cout << "=====================++++++++++++++++++++++++++++++++++++++++++++++++++++++ ===========================\n";
    cout << "===================== FIN DU TRAITEMENT PAR DEEP-FIRST / ROUND ROBIN NELSON ===========================\n";
    cout << "=====================++++++++++++++++++++++++++++++++++++++++++++++++++++++ ===========================\n";
}

// ============================================================
//  ALGORITHME 2 — Dijkstra (terminal)
// ============================================================
static void traiterDijkstra(const string& chemin, const string& label) {
    separateur(label);

    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(chemin)) {
        cerr << "  [ERREUR] Impossible de charger : " << chemin << "\n";
        return;
    }
    fm.afficher();

    auto debut = high_resolution_clock::now();

    AlgorithmeDijkstra algo(&fm);
    ResultatDijkstra   res = algo.executer();

    auto fin     = high_resolution_clock::now();
    long long us = duration_cast<microseconds>(fin - debut).count();

    algo.afficherResultat(res);
    cout << "  [PERF] Temps total (chargement + algo) : " << us << " us\n";

    if (!res.cheminTrouve) {
        cout << "  [INFO] Aucun chemin Sv -> Sd trouve.\n";
        return;
    }

    const Salle* dortoir   = fm.getDortoir();
    int          nbArrivee = 0;
    for (const Fourmi* f : fm.getFourmis())
        if (f->getSalleActuelle() == dortoir)
            nbArrivee++;

    cout << "  [INFO] Fourmis au dortoir : "
         << nbArrivee << " / " << fm.getNbFourmis() << "\n";
}

static void lancerDijkstra() {
    cout << "\n================================================\n";
    cout << "   SIMULATION DES FOURMILIERES — DIJKSTRA      \n";
    cout << "================================================\n";

    const vector<pair<string, string>> fourmilieres = {
        { BASE + "fourmiliere_zero.txt",           "Fourmiliere 0 (exemple de base)" },
        { BASE + "fourmiliere_un.txt",             "Fourmiliere 1"                   },
        { BASE + "fourmiliere_deux.txt",           "Fourmiliere 2"                   },
        { BASE + "fourmiliere_trois.txt",          "Fourmiliere 3"                   },
        { BASE + "fourmiliere_quatre.txt",         "Fourmiliere 4"                   },
        { BASE + "fourmiliere_cinq.txt",           "Fourmiliere 5"                   },
        { BASE + "salle_d_at-ant.txt",             "Salle d'at-ant"                  },
        { BASE + "La_hormiguera_de_la_muerte.txt", "La Hormiguera de la Muerte"       },
    };

    for (const auto& [chemin, label] : fourmilieres)
        traiterDijkstra(chemin, label);

    cout << "\n================================================\n";
    cout << "   FIN DE SIMULATION — DIJKSTRA\n";
    cout << "================================================\n";
}

// ============================================================
//  ALGORITHME 3 — Visualiseur SFML (fenêtre graphique)
// ============================================================
static void lancerVisualiseur() {
    cout << "\n  [INFO] Ouverture de la fenetre graphique...\n";
    cout << "  [INFO] Le terminal reste actif. Fermez la fenetre pour revenir au menu.\n\n";

    const vector<string> fichiers = {
        BASE + "fourmiliere_zero.txt",
        BASE + "fourmiliere_un.txt",
        BASE + "fourmiliere_deux.txt",
        BASE + "fourmiliere_trois.txt",
        BASE + "fourmiliere_quatre.txt",
        BASE + "fourmiliere_cinq.txt",
        BASE + "salle_d_at-ant.txt",
        BASE + "La_hormiguera_de_la_muerte.txt",
    };

    Visualiseur vis(1280, 800);
    vis.run(fichiers);  // bloquant jusqu'à fermeture de la fenêtre

    cout << "\n  [INFO] Fenetre fermee, retour au menu.\n";
}

// ============================================================
//  Menu interactif
// ============================================================
static void afficherMenu() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║        SIMULATION DE FOURMILIERES            ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  1  —  Deep First + Round Robin Nelson       ║\n";
    cout << "║  2  —  Dijkstra                              ║\n";
    cout << "║  3  —  Dijkstra+deep interface               ║\n";
    cout << "║  0  —  Quitter                               ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "  Votre choix : ";
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
            cout << "  [ERREUR] Entree invalide, veuillez saisir 0, 1, 2 ou 3.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choix) {
            case 1:  lancerDeepFirst();    break;
            case 2:  lancerDijkstra();     break;
            case 3:  lancerVisualiseur();  break;
            case 0:  cout << "\n  Au revoir !\n\n"; break;
            default: cout << "  [ERREUR] Choix invalide, veuillez saisir 0, 1, 2 ou 3.\n"; break;
        }
    }

    return 0;
}