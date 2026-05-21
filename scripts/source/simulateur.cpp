#include "../include/simulateur.hpp"
#include <iostream>
#include <climits>

Simulateur::Simulateur(const Fourmiliere& fourmiliere)
    : fourmiliere(fourmiliere), numeroEtape(0) {}

// Orchestre la simulation avec détection de deadlock :
// si aucune fourmi ne bouge pendant une étape, on est bloqué définitivement
void Simulateur::simuler() {
    _etapes.clear();
    chemins = AlgoDeepFirst::trouverTousLesChemins(fourmiliere);

    if (chemins.empty()) {
        std::cout << "Aucun chemin Sv -> Sd trouvé." << std::endl;
        return;
    }

    assignerFourmisALeursChemins();

    while (!toutesAuDortoir()) {
        numeroEtape++;
        int nbMouvements = 0;
        executerUneEtape(nbMouvements);

        // Deadlock : plus aucune fourmi ne peut avancer
        if (nbMouvements == 0) {
            std::cout << "[DEADLOCK] Aucun mouvement possible a l'etape "
                      << numeroEtape << " — simulation arretee." << std::endl;
            break;
        }
    }
}

const std::vector<std::vector<MouvementEtape>>& Simulateur::getEtapes() const {
    return _etapes;
}

// Débit d'un chemin = capacité de la salle la plus étroite (hors Sv et Sd)
// C'est le nombre max de fourmis pouvant traverser ce chemin simultanément
int Simulateur::calculerDebit(const std::vector<Salle*>& chemin) const {
    int debit = INT_MAX;
    for (int i = 1; i < (int)chemin.size() - 1; i++) {
        int cap = chemin[i]->getCapacite();
        if (cap != Salle::CAPACITE_ILLIMITEE && cap < debit)
            debit = cap;
    }
    return (debit == INT_MAX) ? 1 : debit;
}

// Assignation pondérée par le débit :
// un chemin de débit 3 reçoit 3x plus de fourmis qu'un chemin de débit 1.
// On remplit chaque chemin jusqu'à sa limite avant de passer au suivant.
void Simulateur::assignerFourmisALeursChemins() {
    const std::vector<Fourmi*>& fourmis = fourmiliere.getFourmis();

    // Calcule le débit de chaque chemin
    std::vector<int> debits;
    for (int i = 0; i < (int)chemins.size(); i++)
        debits.push_back(calculerDebit(chemins[i]));

    // Compteur d'assignations par chemin pour respecter le débit
    std::vector<int> assignes(chemins.size(), 0);

    for (int i = 0; i < (int)fourmis.size(); i++) {
        // Cherche le chemin avec le meilleur ratio restant (débit - déjà assignés)
        // parmi ceux qui n'ont pas encore atteint leur plafond de débit
        int cheminChoisi = -1;
        int meilleurReste = -1;

        for (int j = 0; j < (int)chemins.size(); j++) {
            int reste = debits[j] - (assignes[j] % debits[j]);
            if (reste > meilleurReste) {
                meilleurReste = reste;
                cheminChoisi  = j;
            }
        }

        // Fallback : round-robin si tous les chemins sont au même niveau
        if (cheminChoisi == -1)
            cheminChoisi = i % (int)chemins.size();

        assignation[fourmis[i]]       = chemins[cheminChoisi];
        positionSurChemin[fourmis[i]] = 0;
        aPlanifie[fourmis[i]]         = false;
        assignes[cheminChoisi]++;
    }
}

// Exécute une étape : planification puis commit
// nbMouvements permet à simuler() de détecter un deadlock
void Simulateur::executerUneEtape(int& nbMouvements) {
    const std::vector<Fourmi*>& fourmis = fourmiliere.getFourmis();

    for (int i = 0; i < (int)fourmis.size(); i++)
        aPlanifie[fourmis[i]] = false;

    // Phase 1 — planification
    for (int i = 0; i < (int)fourmis.size(); i++) {
        Fourmi* fourmi = fourmis[i];
        if (fourmi->estAuDortoir()) continue;

        std::vector<Salle*>& chemin = assignation[fourmi];
        int pos                     = positionSurChemin[fourmi];
        Salle* prochaine            = chemin[pos + 1];

        aPlanifie[fourmi] = fourmi->planifierDeplacement(prochaine);
    }

    // Phase 2 — commit + collecte des mouvements pour affichage et stockage
    std::vector<std::string>   lignesTerminal;
    std::vector<MouvementEtape> mouvementsEtape;

    for (int i = 0; i < (int)fourmis.size(); i++) {
        Fourmi* fourmi = fourmis[i];
        if (!aPlanifie[fourmi]) continue;

        lignesTerminal.push_back(fourmi->formatDeplacement());
        mouvementsEtape.push_back({ fourmi->getId(), fourmi->getDestination()->getNom() });

        fourmi->commitDeplacement();
        positionSurChemin[fourmi]++;
        nbMouvements++;
    }

    _etapes.push_back(mouvementsEtape);

    std::cout << "===== E" << numeroEtape << " =====" << std::endl;
    for (int i = 0; i < (int)lignesTerminal.size(); i++)
        std::cout << lignesTerminal[i] << std::endl;
}

bool Simulateur::toutesAuDortoir() const {
    const std::vector<Fourmi*>& fourmis = fourmiliere.getFourmis();
    for (int i = 0; i < (int)fourmis.size(); i++) {
        if (!fourmis[i]->estAuDortoir()) return false;
    }
    return true;
}