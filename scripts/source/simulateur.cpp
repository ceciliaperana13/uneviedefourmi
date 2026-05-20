#include "../include/simulateur.hpp"

// Reçoit la fourmilière déjà construite et initialise le compteur d'étapes
Simulateur::Simulateur(const Fourmiliere& fourmiliere)
    : fourmiliere(fourmiliere), numeroEtape(0) {}

// trouve les chemins, assigne les fourmis, boucle jusqu'à ce que tout le monde soit au dortoir
void Simulateur::simuler() {
    chemins = AlgoDeepFirst::trouverTousLesChemins(fourmiliere);
    assignerFourmisALeursChemins();

    while (!toutesAuDortoir()) {
        numeroEtape++;
        executerUneEtape();
    }
}

// Assigne chaque fourmi à un chemin et initialise sa position à 0 (vestibule)
void Simulateur::assignerFourmisALeursChemins() {
    const vector<Fourmi*>& fourmis = fourmiliere.getFourmis();

    for (int i = 0; i < (int)fourmis.size(); i++) {
        assignation[fourmis[i]]       = chemins[i % chemins.size()];
        positionSurChemin[fourmis[i]] = 0;
        aPlanifie[fourmis[i]]         = false;
    }
}

// Exécute une étape complète en deux phases : planification puis commit
// Affiche ensuite les mouvements de l'étape
void Simulateur::executerUneEtape() {
    const vector<Fourmi*>& fourmis = fourmiliere.getFourmis();

    // Réinitialise les flags : une fourmi qui a bougé au tour précédent
    // ne doit pas être commitée à nouveau sans avoir replanifié
    for (Fourmi* fourmi : fourmis)
        aPlanifie[fourmi] = false;

    // Planification : chaque fourmi réserve sa prochaine salle et bloque une place
    for (Fourmi* fourmi : fourmis) {
        if (fourmi->estAuDortoir()) continue;

        vector<Salle*>& chemin = assignation[fourmi];
        int pos                     = positionSurChemin[fourmi];
        Salle* prochaine            = chemin[pos + 1];

        aPlanifie[fourmi] = fourmi->planifierDeplacement(prochaine);
    }

    // Commit : on récupère le texte du mouvement AVANT de bouger
    // car commitDeplacement() remet prochaineSalle à nullptr
    vector<string> mouvements;

    for (Fourmi* fourmi : fourmis) {
        if (!aPlanifie[fourmi]) continue;

        mouvements.push_back(fourmi->formatDeplacement());
        fourmi->commitDeplacement();
        positionSurChemin[fourmi]++;
    }

    // Affichage de l'étape
    cout << "===== E" << numeroEtape << " =====" << endl;
    for (const string& mouvement : mouvements)
        cout << mouvement << endl;
}

// Retourne true uniquement si toutes les fourmis ont atteint le dortoir
bool Simulateur::toutesAuDortoir() const {
    for (Fourmi* fourmi : fourmiliere.getFourmis()) {
        if (!fourmi->estAuDortoir()) return false;
    }
    return true;
}