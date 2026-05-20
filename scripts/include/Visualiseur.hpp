#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <string>
#include "fourmiliere.hpp"
#include "Dijkstra.hpp"

using namespace std;

// ============================================================
//  Position 2D d'une salle dans la fenêtre
// ============================================================
struct PositionSalle {
    float x, y;
};

// ============================================================
//  Visualiseur — fenêtre SFML animée
// ============================================================
class Visualiseur {
public:
    Visualiseur(unsigned int largeur = 1280, unsigned int hauteur = 800);
    ~Visualiseur() = default;

    // Charge et affiche les N fourmilières en boucle
    void run(const vector<string>& cheminsFichiers);

private:
    // ---- Chargement ----
    void chargerFourmiliere(const string& chemin);
    void calculerPositions();
    void initialiserEtapes();

    // ---- Rendu ----
    void dessinerGraphe();
    void dessinerTunnels();
    void dessinerSalles();
    void dessinerFourmis();
    void dessinerPanneau();   // infos à droite

    // ---- Animation ----
    void avancerEtape();
    void reculerEtape();
    void reinitialiser();

    // ---- Helpers ----
    sf::Vector2f positionSalle(const string& nom) const;
    sf::Color    couleurFourmi(int id) const;
    string       nomFourmiliere(int index) const;

    // ---- SFML ----
    sf::RenderWindow        _fenetre;
    sf::Font                _police;
    unsigned int            _largeur, _hauteur;

    // ---- Données fourmilière courante ----
    Fourmiliere*            _fm;
    ResultatDijkstra        _res;
    map<string, PositionSalle> _positions;

    // ---- État de l'animation ----
    // Chaque étape = vecteur de (fourmi_id, nom_salle_destination)
    struct MouvementEtape {
        int    fourmiId;
        string salleDest;
    };
    vector<vector<MouvementEtape>> _etapes;
    // Position courante de chaque fourmi (nom de salle)
    map<int, string>               _posFourmis;
    int                            _etapeCourante;  // -1 = état initial

    // ---- Navigation fourmilières ----
    vector<string>  _fichiers;
    int             _indexFm;
};