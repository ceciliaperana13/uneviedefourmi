#include "../include/menu_principal.hpp"

MenuPrincipal::MenuPrincipal(const std::string& dossierFourmilieres)
    : _fichiers(listerFichiersTxt(dossierFourmilieres))
{
    if (_fichiers.empty())
        std::cerr << "[WARN] Aucun fichier .txt trouve dans : " << dossierFourmilieres << "\n";
}

void MenuPrincipal::run() {
    int choix = -1;
    while (choix != 0) {
        afficherMenu();
        if (!(std::cin >> choix)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "[ERREUR] Entree invalide.\n";
            continue;
        }
        switch (choix) {
            case 1:  lancerDeepFirst();               break;
            case 2:  lancerDijkstra();                break;
            case 3:  lancerVisualiseur();             break;
            case 0:  std::cout << "Au revoir !\n";   break;
            default: std::cout << "[ERREUR] Choix invalide.\n"; break;
        }
    }
}

void MenuPrincipal::afficherMenu() const {
    std::cout << "\n";
    std::cout << "+----------------------------------------------+\n";
    std::cout << "|        SIMULATION DE FOURMILIERES            |\n";
    std::cout << "+----------------------------------------------+\n";
    std::cout << "|  1  -  Deep First + Round Robin Nelson       |\n";
    std::cout << "|  2  -  Dijkstra                              |\n";
    std::cout << "|  3  -  Visualiseur graphique                 |\n";
    std::cout << "|  0  -  Quitter                               |\n";
    std::cout << "+----------------------------------------------+\n";
    std::cout << "Votre choix : ";
}

std::string MenuPrincipal::choisirFourmiliere() const {
    if (_fichiers.empty()) {
        std::cout << "[ERREUR] Aucune fourmiliere disponible.\n";
        return "";
    }

    std::cout << "\nChoisissez une fourmiliere :\n";
    for (int i = 0; i < (int)_fichiers.size(); i++)
        std::cout << "  " << i << " - " << nomAffichable(_fichiers[i]) << "\n";
    std::cout << "Votre choix : ";

    int choix = -1;
    std::cin >> choix;

    if (choix < 0 || choix >= (int)_fichiers.size()) {
        std::cout << "[ERREUR] Choix invalide.\n";
        return "";
    }
    return _fichiers[choix];
}

void MenuPrincipal::lancerDeepFirst() const {
    std::string fichier = choisirFourmiliere();
    if (fichier.empty()) return;

    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(fichier)) {
        std::cerr << "[ERREUR] Impossible de charger " << fichier << "\n";
        return;
    }
    fm.afficher();

    Simulateur sim(fm);
    sim.simuler();
}

void MenuPrincipal::lancerDijkstra() const {
    std::string fichier = choisirFourmiliere();
    if (fichier.empty()) return;

    Fourmiliere fm;
    if (!fm.chargerDepuisFichier(fichier)) {
        std::cerr << "[ERREUR] Impossible de charger " << fichier << "\n";
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

std::string MenuPrincipal::nomAffichable(const std::string& chemin) {
    namespace fs = std::filesystem;
    return fs::path(chemin).stem().string();
}

std::vector<std::string> MenuPrincipal::listerFichiersTxt(const std::string& dossier) {
    std::vector<std::string> fichiers;
    namespace fs = std::filesystem;

    if (!fs::exists(dossier) || !fs::is_directory(dossier)) {
        std::cerr << "[ERREUR] Dossier introuvable : " << dossier << "\n";
        return fichiers;
    }

    for (const auto& entry : fs::directory_iterator(dossier)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
            fichiers.push_back(entry.path().generic_string());
    }

    std::sort(fichiers.begin(), fichiers.end());
    return fichiers;
}