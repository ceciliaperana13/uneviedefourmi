#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <string>
#include "fourmiliere.hpp"
#include "Dijkstra.hpp"
#include "algo_deep_first.hpp"
#include "simulateur.hpp"

using namespace std;

struct PositionSalle {
    float x, y;
};

enum class ModeAnimation { DIJKSTRA, DFS };

class Visualiseur {
public:
    Visualiseur(unsigned int largeur = 1280, unsigned int hauteur = 800);
    ~Visualiseur() = default;

    void run(const vector<string>& cheminsFichiers);

private:
    void chargerFourmiliere(const string& chemin);
    void calculerPositions();
    void initialiserEtapesDijkstra();

    void dessinerTunnels();
    void dessinerSalles();
    void dessinerFourmis();
    void dessinerPanneau();

    void avancerEtape();
    void reculerEtape();
    void reinitialiser();
    void basculerMode();

    sf::Vector2f positionSalle(const string& nom) const;
    sf::Color    couleurFourmi(int id) const;
    string       nomFourmiliere(int index) const;

    const vector<vector<MouvementEtape>>& etapesActives() const;

    // SFML
    sf::RenderWindow _fenetre;
    sf::Font         _police;
    unsigned int     _largeur, _hauteur;

    // Données
    Fourmiliere*               _fm;
    ResultatDijkstra           _resDijkstra;       // chemin optimal + distances + nbTours
    vector<vector<Salle*>>     _cheminsDFS;        // tous les chemins DFS
    long long                  _tempsDfsUs  = 0;   // durée CPU DFS en µs
    int                        _tempsSimSec = 0;   // nbTours = secondes Sv→Sd
    map<string, PositionSalle> _positions;

    vector<vector<MouvementEtape>> _etapesDijkstra;
    vector<vector<MouvementEtape>> _etapesDFS;
    map<int, string>               _posFourmis;
    int                            _etapeCourante;

    ModeAnimation  _mode;
    vector<string> _fichiers;
    int            _indexFm;
};