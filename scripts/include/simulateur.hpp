#pragma once
#include <map>
#include <string>
#include <vector>
#include "algo_deep_first.hpp"
#include "fourmi.hpp"
#include "fourmiliere.hpp"
#include "salle.hpp"

struct MouvementEtape {
    int         fourmiId;
    string salleDest;
};

class Simulateur {
public:
    Simulateur(const Fourmiliere& fourmiliere);
    void simuler();

    const vector<vector<MouvementEtape>>& getEtapes() const;

private:
    const Fourmiliere&                     fourmiliere;
    vector<vector<Salle*>>       chemins;
    map<Fourmi*, vector<Salle*>> assignation;
    map<Fourmi*, int>                 positionSurChemin;
    map<Fourmi*, bool>                aPlanifie;
    int                                    numeroEtape;

    vector<vector<MouvementEtape>> _etapes;

    // Débit = min des capacités des salles intermédiaires du chemin
    // Limite le nombre de fourmis pouvant avancer simultanément
    int calculerDebit(const vector<Salle*>& chemin) const;

    // Temps estimé pour écouler nbFourmis sur un chemin en pipeline :
    // (nb_tunnels - 1) + ceil(nbFourmis / debit)
    // Permet de comparer des chemins de longueurs et débits différents
    int tempsEstime(int nbFourmis, const vector<Salle*>& chemin) const;

    void assignerFourmisALeursChemins();
    void executerUneEtape(int& nbMouvements);
    bool toutesAuDortoir() const;
};