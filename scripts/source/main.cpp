#include "../include/fourmiliere.hpp"
#include "../include/Dijkstra.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

// ============================================================
//  Affiche un séparateur lisible
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
//  Traite une fourmilière : charge, lance Dijkstra, affiche
// ============================================================
static void traiterFourmiliere(const string& chemin, const string& label) {
    separateur(label);

    // --- Chargement ---
    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(chemin)) {
        cerr << "  [ERREUR] Impossible de charger : " << chemin << "\n";
        return;
    }
    fm.afficher();

    // --- Dijkstra ---
    auto debut = high_resolution_clock::now();

    AlgorithmeDijkstra algo(&fm);
    ResultatDijkstra   res = algo.executer();

    auto fin    = high_resolution_clock::now();
    long long us = duration_cast<microseconds>(fin - debut).count();

    // --- Résultat ---
    algo.afficherResultat(res);
    cout << "  [PERF] Temps total (chargement + algo) : " << us << " us\n";

    if (!res.cheminTrouve) {
        cout << "  [INFO] Aucun chemin Sv -> Sd trouvé.\n";
        return;
    }

    // --- Vérification : toutes les fourmis au dortoir ? ---
    const Salle* dortoir = fm.getDortoir();
    int nbArrivees = 0;
    for (const Fourmi* f : fm.getFourmis()) {
        if (f->getSalleActuelle() == dortoir)
            nbArrivees++;
    }
    cout << "  [INFO] Fourmis au dortoir : "
         << nbArrivees << " / " << fm.getNbFourmis() << "\n";
}

// ============================================================
//  Main — 5 fourmilières + fourmilière zéro (exemple de base)
// ============================================================
int main() {
    cout << "================================================\n";
    cout << "   SIMULATION DES FOURMILIERES — DIJKSTRA      \n";
    cout << "================================================\n";

    // Chemin relatif depuis le répertoire d'exécution (scripts/)
    // Adaptez si vous lancez depuis la racine du projet
    const string BASE = "../fourmilieres/";

    const vector<pair<string,string>> fourmilieres = {
        { BASE + "fourmiliere_zero.txt",  "Fourmiliere 0 (exemple de base)" },
        { BASE + "fourmiliere_un.txt",    "Fourmiliere 1"                   },
        { BASE + "fourmiliere_deux.txt",  "Fourmiliere 2"                   },
        { BASE + "fourmiliere_trois.txt", "Fourmiliere 3"                   },
        { BASE + "fourmiliere_quatre.txt","Fourmiliere 4"                   },
        { BASE + "fourmiliere_cinq.txt",  "Fourmiliere 5"                   },
    };

    for (const auto& [chemin, label] : fourmilieres)
        traiterFourmiliere(chemin, label);

    cout << "\n================================================\n";
    cout << "   FIN DE SIMULATION\n";
    cout << "================================================\n";

    return 0;
}