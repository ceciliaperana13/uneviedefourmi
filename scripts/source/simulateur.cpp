#include "../include/simulateur.hpp"
#include <iostream>
#include <climits>
#include <cmath>
#include <algorithm>

Simulateur::Simulateur(const Fourmiliere& fourmiliere)
    : fourmiliere(fourmiliere), numeroEtape(0) {}

void Simulateur::simuler() {
    _etapes.clear();
    chemins = AlgoDeepFirst::trouverTousLesChemins(fourmiliere);

    if (chemins.empty()) {
        cout << "Aucun chemin Sv -> Sd trouve." << endl;
        return;
    }

    assignerFourmisALeursChemins();

    while (!toutesAuDortoir()) {
        numeroEtape++;
        int nbMouvements = 0;
        executerUneEtape(nbMouvements);

        if (nbMouvements == 0) {
            cout << "[DEADLOCK] Aucun mouvement possible a l'etape "
                      << numeroEtape << " - simulation arretee." << endl;
            break;
        }
    }
}

const vector<vector<MouvementEtape>>& Simulateur::getEtapes() const {
    return _etapes;
}

int Simulateur::calculerDebit(const vector<Salle*>& chemin) const {
    int debit = INT_MAX;
    for (int i = 1; i < (int)chemin.size() - 1; i++) {
        int cap = chemin[i]->getCapacite();
        if (cap != Salle::CAPACITE_ILLIMITEE && cap < debit)
            debit = cap;
    }
    return (debit == INT_MAX) ? INT_MAX : debit;
}

int Simulateur::tempsEstime(int nbFourmis, const vector<Salle*>& chemin) const {
    int longueur = (int)chemin.size() - 1;
    int debit    = calculerDebit(chemin);
    if (debit == INT_MAX) debit = nbFourmis;
    return longueur + (int)ceil((double)nbFourmis / debit);
}

void Simulateur::assignerFourmisALeursChemins() {
    const vector<Fourmi*>& fourmis = fourmiliere.getFourmis();
    vector<int> assignes(chemins.size(), 0);

    for (int i = 0; i < (int)fourmis.size(); i++) {
        int cheminChoisi  = 0;
        int meilleurTemps = INT_MAX;

        for (int j = 0; j < (int)chemins.size(); j++) {
            int t = tempsEstime(assignes[j] + 1, chemins[j]);
            if (t < meilleurTemps) {
                meilleurTemps = t;
                cheminChoisi  = j;
            }
        }

        assignation[fourmis[i]]       = chemins[cheminChoisi];
        positionSurChemin[fourmis[i]] = 0;
        assignes[cheminChoisi]++;
    }
}

// Pipeline correct : on planifie ET committe immédiatement fourmi par fourmi,
// de la plus avancée à la moins avancée.
// Ainsi quand f36 essaie d'entrer en S4, f40 l'a déjà quittée —
// la capacité libérée est immédiatement visible pour les fourmis derrière.
void Simulateur::executerUneEtape(int& nbMouvements) {
    const vector<Fourmi*>& fourmis = fourmiliere.getFourmis();

    vector<Fourmi*> ordre(fourmis.begin(), fourmis.end());
    sort(ordre.begin(), ordre.end(), [&](Fourmi* a, Fourmi* b) {
        return positionSurChemin[a] > positionSurChemin[b];
    });

    vector<string>    lignesTerminal;
    vector<MouvementEtape> mouvementsEtape;

    for (Fourmi* fourmi : ordre) {
        if (fourmi->estAuDortoir()) continue;

        int    pos      = positionSurChemin[fourmi];
        Salle* prochain = assignation[fourmi][pos + 1];

        // Planifier et committer immédiatement pour libérer la place
        // avant que la fourmi derrière essaie d'avancer
        if (fourmi->planifierDeplacement(prochain)) {
            lignesTerminal.push_back(fourmi->formatDeplacement());
            mouvementsEtape.push_back({ fourmi->getId(), prochain->getNom() });
            fourmi->commitDeplacement();
            positionSurChemin[fourmi]++;
            nbMouvements++;
        }
    }

    _etapes.push_back(mouvementsEtape);

    cout << "===== E" << numeroEtape << " =====" << endl;
    for (const auto& ligne : lignesTerminal)
        cout << ligne << endl;
}

bool Simulateur::toutesAuDortoir() const {
    const vector<Fourmi*>& fourmis = fourmiliere.getFourmis();
    for (Fourmi* f : fourmis)
        if (!f->estAuDortoir()) return false;
    return true;
}