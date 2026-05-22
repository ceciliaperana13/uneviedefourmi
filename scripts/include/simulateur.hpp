#pragma once
#include <map>
#include <string>
#include <vector>
#include "algo_deep_first.hpp"
#include "fourmi.hpp"
#include "fourmiliere.hpp"
#include "salle.hpp"

#ifndef MOUVEMENT_ETAPE_DEFINED
#define MOUVEMENT_ETAPE_DEFINED
struct MouvementEtape {
    int         fourmiId;
    std::string salleDest;
};
#endif

class Simulateur {
public:
    Simulateur(const Fourmiliere& fourmiliere);
    void simuler();
    const std::vector<std::vector<MouvementEtape>>& getEtapes() const;
private:
    const Fourmiliere&                       fourmiliere;
    std::vector<std::vector<Salle*>>         chemins;
    std::map<Fourmi*, std::vector<Salle*>>   assignation;
    std::map<Fourmi*, int>                   positionSurChemin;
    std::map<Fourmi*, bool>                  aPlanifie;
    int                                      numeroEtape;
    std::vector<std::vector<MouvementEtape>> _etapes;

    int  calculerDebit(const std::vector<Salle*>& chemin) const;
    int  tempsEstime(int nbFourmis, const std::vector<Salle*>& chemin) const;
    void assignerFourmisALeursChemins();
    void executerUneEtape(int& nbMouvements);
    bool toutesAuDortoir() const;
};