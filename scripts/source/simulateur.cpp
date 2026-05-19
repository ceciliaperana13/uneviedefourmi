#include "../include/simulateur.hpp"

// PSEUDOCODE DE L'ALGORITHME DEEP-FIRST

// 1. Trouver tous les chemins (DFS), et établir leur capacité en fourmis par étape
// 2. Assigner chaque fourmi à un chemin
// 3. Boucle par étape :
//    a. Pour chaque fourmi pas au dortoir :
//       → planifierDeplacement(prochaine salle sur son chemin)
//    b. Commiter tous les mouvements planifiés
//    c. Afficher l'étape
// 4. Répéter jusqu'à ce que toutes les fourmis soient au dortoir

vector<vector<Salle*>> établirLesCheminsParDeepFirst() { 

    vector<vector<Salle*>> lesChemins;
    return lesChemins;

}

void assignerFourmisALeursChemins(vector<Fourmi> fourmis, vector<vector<Salle*>> chemins) { 
    for (int i = 0; i < fourmis.size(); i++) {

        assignation[fourmis[i]]      = chemins[i % chemins.size()];

        positionSurChemin[fourmis[i]] = 0;
    }
}

void executerUneEtape() { 

    // Réinitialiser le flag pour toutes les fourmis 
    for (Fourmi* fourmi : fourmis) {
        aPlanifie[fourmi] = false;
    }

    // Pour chaque fourmi pas au dortoir :
    for (Fourmi* fourmi : fourmis) {

        // Ignore les fourmis déjà arrivées 
        if (fourmi->estAuDortoir()) continue;

        // 1. Je récupère le chemin de cette fourmi
        vector<Salle*>& chemin = assignation[fourmi];

        // 2. Je récupère sa position actuelle
        int pos = positionSurChemin[fourmi];

        // 3. La prochaine salle c'est
        Salle* prochaine = chemin[pos + 1];

        aPlanifie[fourmi] = fourmi->planifierDeplacement(prochaine);

    }

    // Commit de tous les mouvements planifiés
    for (Fourmi* fourmi : fourmis) {

        vector<string> mouvements;

        // Boucle de commit : on traite chaque fourmi qui a planifié un mouvement
        for (Fourmi* fourmi : fourmis) {

            // Si cette fourmi n'a pas planifié de mouvement, on la saute
            if (!aPlanifie[fourmi]) continue;

            // 1. On récupère le texte du mouvement AVANT le commit
            //    car après, prochaineSalle sera remis à nullptr
            mouvements.push_back(fourmi->formatDeplacement());

            // 2. On effectue le déplacement physique
            //    → salleActuelle devient prochaineSalle
            //    → prochaineSalle revient à nullptr
            fourmi->commitDeplacement();

            // 3. On avance la position de la fourmi sur son chemin
            //    → elle pointera vers la bonne prochaine salle à l'étape suivante
            positionSurChemin[fourmi]++;
        }

        // Affichage de l'étape : numéro puis chaque mouvement
        cout << "+++++ E " << numeroEtape << " ++++++" << endl;
        for (const string& mouvement : mouvements) {
            cout << mouvement << endl;
}

}

}

bool toutesAuDortoir(vector<Fourmi> fourmis) { 
    for (Fourmi* fourmi : fourmis) {
        if (!fourmi->estAuDortoir()) {
            return false;
        }
    }
    return true; 
}

void simuler() {
    chemins   = établirLesCheminsParDeepFirst();
    assignerFourmisALeursChemins(fourmis, chemins);

    while (!toutesAuDortoir()) {
        executerUneEtape();
    }
}

