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
    std::string salleDest;
};

class Simulateur {
public:
    Simulateur(const Fourmiliere& fourmiliere);
    void simuler();

    const std::vector<std::vector<MouvementEtape>>& getEtapes() const;

private:
    const Fourmiliere&                     fourmiliere;
    std::vector<std::vector<Salle*>>       chemins;
    std::map<Fourmi*, std::vector<Salle*>> assignation;
    std::map<Fourmi*, int>                 positionSurChemin;
    std::map<Fourmi*, bool>                aPlanifie;
    int                                    numeroEtape;

    std::vector<std::vector<MouvementEtape>> _etapes;

    // Débit = min des capacités des salles intermédiaires du chemin
    // Limite le nombre de fourmis assignables à ce chemin
    int calculerDebit(const std::vector<Salle*>& chemin) const;

    void assignerFourmisALeursChemins();
    void executerUneEtape(int& nbMouvements);
    bool toutesAuDortoir() const;
};