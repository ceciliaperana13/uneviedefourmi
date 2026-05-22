#include "../include/menu_principal.hpp"

MenuPrincipal::MenuPrincipal(const string& dossierFourmilieres)
    : _fichiers(listerFichiersTxt(dossierFourmilieres))
{
    if (_fichiers.empty())
        cerr << "[WARN] Aucun fichier .txt trouve dans : " << dossierFourmilieres << "\n";
}

void MenuPrincipal::run() {
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
            case 1:  lancerDeepFirst();               break;
            case 2:  lancerDijkstra();                break;
            case 3:  lancerVisualiseur();             break;
            case 0:  cout << "Au revoir !\n";   break;
            default: cout << "[ERREUR] Choix invalide.\n"; break;
        }
    }
}

void MenuPrincipal::afficherMenu() const {
    cout << "\n";
    cout << "+----------------------------------------------+\n";
    cout << "|        SIMULATION DE FOURMILIERES            |\n";
    cout << "+----------------------------------------------+\n";
    cout << "|  1  -  Deep First + Round Robin Nelson       |\n";
    cout << "|  2  -  Dijkstra                              |\n";
    cout << "|  3  -  Visualiseur graphique                 |\n";
    cout << "|  0  -  Quitter                               |\n";
    cout << "+----------------------------------------------+\n";
    cout << "Votre choix : ";
}

string MenuPrincipal::choisirFourmiliere() const {
    if (_fichiers.empty()) {
        cout << "[ERREUR] Aucune fourmiliere disponible.\n";
        return "";
    }

    cout << "\nChoisissez une fourmiliere :\n";
    for (int i = 0; i < (int)_fichiers.size(); i++)
        cout << "  " << i << " - " << nomAffichable(_fichiers[i]) << "\n";
    cout << "Votre choix : ";

    int choix = -1;
    cin >> choix;

    if (choix < 0 || choix >= (int)_fichiers.size()) {
        cout << "[ERREUR] Choix invalide.\n";
        return "";
    }
    return _fichiers[choix];
}

void MenuPrincipal::lancerDeepFirst() const {
    string fichier = choisirFourmiliere();
    if (fichier.empty()) return;

    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(fichier)) {
        cerr << "[ERREUR] Impossible de charger " << fichier << "\n";
        return;
    }
    fm.afficher();

    Simulateur sim(fm);
    sim.simuler();
}

void MenuPrincipal::lancerDijkstra() const {
    string fichier = choisirFourmiliere();
    if (fichier.empty()) return;

    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(fichier)) {
        cerr << "[ERREUR] Impossible de charger " << fichier << "\n";
        return;
    }
    fm.afficher();

    AlgorithmeDijkstra algo(&fm);
    ResultatDijkstra   res = algo.executer();
    algo.afficherResultat(res);
}

void MenuPrincipal::lancerVisualiseur() const {
    Visualiseur vis(1280, 800);
    vis.run(_fichiers);
}

string MenuPrincipal::nomAffichable(const string& chemin) {
    namespace fs = filesystem;
    return fs::path(chemin).stem().string();
}

vector<string> MenuPrincipal::listerFichiersTxt(const string& dossier) {
    vector<string> fichiers;
    namespace fs = filesystem;

    if (!fs::exists(dossier) || !fs::is_directory(dossier)) {
        cerr << "[ERREUR] Dossier introuvable : " << dossier << "\n";
        return fichiers;
    }

    for (const auto& entry : fs::directory_iterator(dossier)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
            fichiers.push_back(entry.path().generic_string());
    }

    sort(fichiers.begin(), fichiers.end());
    return fichiers;
}