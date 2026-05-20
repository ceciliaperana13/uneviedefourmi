#include "../include/Visualiseur.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

// ============================================================
//  Palette de couleurs pour les fourmis (cyclique)
// ============================================================
static const sf::Color PALETTE[] = {
    {220, 80,  80 }, {80,  180, 80 }, {80,  120, 220},
    {220, 160, 40 }, {160, 80,  200}, {40,  180, 200},
    {220, 120, 60 }, {100, 200, 120}, {180, 80,  140},
    {80,  140, 100}
};
static constexpr int NB_COULEURS = 10;

// ============================================================
//  Constructeur
// ============================================================
Visualiseur::Visualiseur(unsigned int largeur, unsigned int hauteur)
    : _fenetre(sf::VideoMode(largeur, hauteur), "Fourmiliere - Dijkstra",
               sf::Style::Titlebar | sf::Style::Close),
      _largeur(largeur), _hauteur(hauteur),
      _fm(nullptr), _etapeCourante(-1), _indexFm(0)
{
    _fenetre.setFramerateLimit(60);

    // Cherche une police système disponible
    vector<string> polices = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc"
    };
    for (const auto& p : polices) {
        if (_police.loadFromFile(p)) break;
    }
}

// ============================================================
//  run — boucle principale
// ============================================================
void Visualiseur::run(const vector<string>& fichiers) {
    _fichiers = fichiers;
    _indexFm  = 0;
    chargerFourmiliere(_fichiers[0]);

    while (_fenetre.isOpen()) {
        sf::Event ev;
        while (_fenetre.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed)
                _fenetre.close();

            if (ev.type == sf::Event::KeyPressed) {
                switch (ev.key.code) {
                    // Navigation étapes
                    case sf::Keyboard::Right:
                    case sf::Keyboard::Space:
                        avancerEtape(); break;
                    case sf::Keyboard::Left:
                        reculerEtape(); break;
                    case sf::Keyboard::R:
                        reinitialiser(); break;
                    // Navigation fourmilières
                    case sf::Keyboard::Up:
                    case sf::Keyboard::PageUp:
                        _indexFm = (_indexFm + 1) % (int)_fichiers.size();
                        chargerFourmiliere(_fichiers[_indexFm]); break;
                    case sf::Keyboard::Down:
                    case sf::Keyboard::PageDown:
                        _indexFm = (_indexFm - 1 + (int)_fichiers.size()) % (int)_fichiers.size();
                        chargerFourmiliere(_fichiers[_indexFm]); break;
                    case sf::Keyboard::Escape:
                        _fenetre.close(); break;
                    default: break;
                }
            }
        }

        _fenetre.clear(sf::Color(30, 30, 40));
        dessinerGraphe();
        dessinerPanneau();
        _fenetre.display();
    }

    delete _fm;
}

// ============================================================
//  Chargement d'une fourmilière
// ============================================================
void Visualiseur::chargerFourmiliere(const string& chemin) {
    delete _fm;
    _fm = new Fourmiliere();

    if (!_fm->chargerDepuisFichier(chemin)) {
        cerr << "Impossible de charger : " << chemin << "\n";
        return;
    }

    // Calcul Dijkstra
    AlgorithmeDijkstra algo(_fm);
    _res = algo.executer();

    calculerPositions();
    initialiserEtapes();
    reinitialiser();
}

// ============================================================
//  Positionnement automatique des salles en cercle
//  Sv à gauche, Sd à droite, les autres en ellipse au centre
// ============================================================
void Visualiseur::calculerPositions() {
    _positions.clear();

    // Zone graphe = 75% de la largeur (panneau à droite = 25%)
    float zoneW = _largeur * 0.72f;
    float zoneH = _hauteur * 0.85f;
    float offsetX = 30.f;
    float offsetY = (_hauteur - zoneH) / 2.f;

    // Vestibule à gauche, dortoir à droite
    _positions["Sv"] = { offsetX + 60.f,          offsetY + zoneH / 2.f };
    _positions["Sd"] = { offsetX + zoneW - 60.f,  offsetY + zoneH / 2.f };

    // Autres salles en ellipse
    const auto& salles = _fm->getSalles();
    vector<string> autres;
    for (const auto& [nom, _] : salles)
        if (nom != "Sv" && nom != "Sd")
            autres.push_back(nom);

    int n = (int)autres.size();
    if (n == 0) return;

    float cx = offsetX + zoneW / 2.f;
    float cy = offsetY + zoneH / 2.f;
    float rx = zoneW * 0.32f;
    float ry = zoneH * 0.38f;

    for (int i = 0; i < n; i++) {
        float angle = (2.f * 3.14159f * i) / n - 3.14159f / 2.f;
        _positions[autres[i]] = {
            cx + rx * cosf(angle),
            cy + ry * sinf(angle)
        };
    }
}

// ============================================================
//  Construction des étapes d'animation depuis le chemin Dijkstra
//  Règle : les fourmis avancent sur le chemin optimal une par une,
//          décalées d'une étape pour éviter les collisions de salles
// ============================================================
void Visualiseur::initialiserEtapes() {
    _etapes.clear();

    if (_res.chemin.empty()) return;

    // Chemin sous forme de noms
    vector<string> chemin;
    for (Salle* s : _res.chemin)
        chemin.push_back(s->getNom());

    int nbFourmis = _fm->getNbFourmis();
    int longueur  = (int)chemin.size(); // inclut Sv et Sd

    // Nombre total d'étapes = longueur chemin - 1 + décalage (nbFourmis-1)
    int nbEtapes = (longueur - 1) + (nbFourmis - 1);
    _etapes.resize(nbEtapes);

    for (int f = 0; f < nbFourmis; f++) {
        int fourmiId = f + 1;
        // La fourmi f part à l'étape f (décalage d'une étape par fourmi)
        for (int step = 0; step < longueur - 1; step++) {
            int etapeIndex = f + step;
            if (etapeIndex < nbEtapes) {
                _etapes[etapeIndex].push_back({
                    fourmiId,
                    chemin[step + 1]  // destination = salle suivante sur le chemin
                });
            }
        }
    }
}

// ============================================================
//  Réinitialise toutes les fourmis au vestibule
// ============================================================
void Visualiseur::reinitialiser() {
    _etapeCourante = -1;
    _posFourmis.clear();
    int nbFourmis = _fm->getNbFourmis();
    for (int i = 1; i <= nbFourmis; i++)
        _posFourmis[i] = "Sv";
}

// ============================================================
//  Avancer / reculer d'une étape
// ============================================================
void Visualiseur::avancerEtape() {
    int maxEtape = (int)_etapes.size() - 1;
    if (_etapeCourante >= maxEtape) return;

    _etapeCourante++;
    // Applique les mouvements de cette étape
    for (const auto& mv : _etapes[_etapeCourante])
        _posFourmis[mv.fourmiId] = mv.salleDest;
}

void Visualiseur::reculerEtape() {
    if (_etapeCourante < 0) return;

    // Recalcule depuis le début jusqu'à etapeCourante - 1
    _etapeCourante--;
    _posFourmis.clear();
    int nbFourmis = _fm->getNbFourmis();
    for (int i = 1; i <= nbFourmis; i++)
        _posFourmis[i] = "Sv";

    for (int e = 0; e <= _etapeCourante; e++)
        for (const auto& mv : _etapes[e])
            _posFourmis[mv.fourmiId] = mv.salleDest;
}

// ============================================================
//  Helpers
// ============================================================
sf::Vector2f Visualiseur::positionSalle(const string& nom) const {
    auto it = _positions.find(nom);
    if (it == _positions.end()) return {0, 0};
    return {it->second.x, it->second.y};
}

sf::Color Visualiseur::couleurFourmi(int id) const {
    return PALETTE[(id - 1) % NB_COULEURS];
}

// ============================================================
//  Dessin principal
// ============================================================
void Visualiseur::dessinerGraphe() {
    if (!_fm) return;
    dessinerTunnels();
    dessinerSalles();
    dessinerFourmis();
}

// ---- Tunnels ----
void Visualiseur::dessinerTunnels() {
    const auto& salles = _fm->getSalles();

    for (const auto& [nom, salle] : salles) {
        sf::Vector2f pA = positionSalle(nom);

        for (const Salle* voisin : salle->getVoisins()) {
            sf::Vector2f pB = positionSalle(voisin->getNom());

            // Tunnel ordinaire en gris
            sf::Color couleur(80, 80, 100);

            // Sur le chemin optimal -> surligné en jaune
            if (!_res.chemin.empty()) {
                for (int i = 0; i + 1 < (int)_res.chemin.size(); i++) {
                    bool ab = (_res.chemin[i]->getNom() == nom &&
                               _res.chemin[i+1]->getNom() == voisin->getNom());
                    bool ba = (_res.chemin[i]->getNom() == voisin->getNom() &&
                               _res.chemin[i+1]->getNom() == nom);
                    if (ab || ba) { couleur = sf::Color(255, 210, 50); break; }
                }
            }

            sf::Vertex ligne[] = {
                sf::Vertex(pA, couleur),
                sf::Vertex(pB, couleur)
            };
            _fenetre.draw(ligne, 2, sf::Lines);
        }
    }
}

// ---- Salles ----
void Visualiseur::dessinerSalles() {
    const auto& salles = _fm->getSalles();
    const float R = 22.f;

    for (const auto& [nom, salle] : salles) {
        sf::Vector2f pos = positionSalle(nom);

        // Cercle
        sf::CircleShape cercle(R);
        cercle.setOrigin(R, R);
        cercle.setPosition(pos);

        bool surChemin = false;
        for (Salle* s : _res.chemin)
            if (s->getNom() == nom) { surChemin = true; break; }

        if (nom == "Sv")
            cercle.setFillColor(sf::Color(60, 160, 100));
        else if (nom == "Sd")
            cercle.setFillColor(sf::Color(60, 100, 200));
        else if (surChemin)
            cercle.setFillColor(sf::Color(80, 80, 50));
        else
            cercle.setFillColor(sf::Color(60, 60, 80));

        cercle.setOutlineThickness(2.f);
        cercle.setOutlineColor(surChemin ? sf::Color(255, 210, 50)
                                         : sf::Color(120, 120, 150));
        _fenetre.draw(cercle);

        // Nom + capacité
        sf::Text texte;
        texte.setFont(_police);
        texte.setCharacterSize(13);
        texte.setFillColor(sf::Color::White);
        texte.setString(nom);
        sf::FloatRect bounds = texte.getLocalBounds();
        texte.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        texte.setPosition(pos.x, pos.y - 4.f);
        _fenetre.draw(texte);

        if (salle->getCapacite() != Salle::CAPACITE_ILLIMITEE) {
            sf::Text cap;
            cap.setFont(_police);
            cap.setCharacterSize(10);
            cap.setFillColor(sf::Color(180, 180, 180));
            cap.setString("c=" + to_string(salle->getCapacite()));
            sf::FloatRect cb = cap.getLocalBounds();
            cap.setOrigin(cb.width / 2.f, 0.f);
            cap.setPosition(pos.x, pos.y + R + 2.f);
            _fenetre.draw(cap);
        }
    }
}

// ---- Fourmis ----
void Visualiseur::dessinerFourmis() {
    // Compte combien de fourmis sont dans chaque salle pour les décaler
    map<string, vector<int>> parSalle;
    for (const auto& [id, salle] : _posFourmis)
        parSalle[salle].push_back(id);

    const float R_FOURMI = 6.f;
    const float OFFSET   = 14.f;

    for (const auto& [salle, ids] : parSalle) {
        sf::Vector2f centre = positionSalle(salle);
        int n = (int)ids.size();

        for (int i = 0; i < n; i++) {
            // Dispose les fourmis en cercle autour du centre de la salle
            float angle = (2.f * 3.14159f * i) / max(n, 1) ;
            float ox = (n == 1) ? 0.f : OFFSET * cosf(angle);
            float oy = (n == 1) ? 0.f : OFFSET * sinf(angle);

            sf::CircleShape pt(R_FOURMI);
            pt.setOrigin(R_FOURMI, R_FOURMI);
            pt.setPosition(centre.x + ox, centre.y + oy);
            pt.setFillColor(couleurFourmi(ids[i]));
            pt.setOutlineThickness(1.f);
            pt.setOutlineColor(sf::Color(255, 255, 255, 80));
            _fenetre.draw(pt);

            // Numéro de la fourmi si pas trop nombreuses
            if (_fm->getNbFourmis() <= 20) {
                sf::Text t;
                t.setFont(_police);
                t.setCharacterSize(8);
                t.setFillColor(sf::Color::White);
                t.setString(to_string(ids[i]));
                sf::FloatRect b = t.getLocalBounds();
                t.setOrigin(b.width/2.f, b.height/2.f);
                t.setPosition(centre.x + ox, centre.y + oy - 1.f);
                _fenetre.draw(t);
            }
        }
    }
}

// ============================================================
//  Panneau d'informations (droite)
// ============================================================
void Visualiseur::dessinerPanneau() {
    float px = _largeur * 0.74f;
    float py = 20.f;
    float pw = _largeur * 0.24f;

    // Fond du panneau
    sf::RectangleShape fond(sf::Vector2f(pw, (float)_hauteur - 40.f));
    fond.setPosition(px - 10.f, 10.f);
    fond.setFillColor(sf::Color(20, 20, 35));
    fond.setOutlineThickness(1.f);
    fond.setOutlineColor(sf::Color(80, 80, 100));
    _fenetre.draw(fond);

    auto texte = [&](const string& s, float x, float y,
                     unsigned int size = 14,
                     sf::Color col = sf::Color::White) {
        sf::Text t;
        t.setFont(_police);
        t.setCharacterSize(size);
        t.setFillColor(col);
        t.setString(s);
        t.setPosition(x, y);
        _fenetre.draw(t);
    };

    // Titre fourmilière
    texte(nomFourmiliere(_indexFm), px, py, 16, sf::Color(255, 210, 50));
    py += 26.f;

    texte("Fourmis : " + to_string(_fm->getNbFourmis()), px, py, 13,
          sf::Color(180, 220, 180));
    py += 20.f;

    // Chemin optimal
    if (!_res.chemin.empty()) {
        texte("Chemin optimal :", px, py, 13, sf::Color(150, 200, 255));
        py += 18.f;
        string ch;
        for (int i = 0; i < (int)_res.chemin.size(); i++) {
            ch += _res.chemin[i]->getNom();
            if (i < (int)_res.chemin.size()-1) ch += "->";
            // Coupe si trop long
            if ((i+1) % 4 == 0 && i < (int)_res.chemin.size()-1) {
                texte("  " + ch, px, py, 12); py += 16.f; ch = "";
            }
        }
        if (!ch.empty()) { texte("  " + ch, px, py, 12); py += 16.f; }
    } else {
        texte("Aucun chemin trouve", px, py, 13, sf::Color(255, 100, 100));
        py += 18.f;
    }

    py += 8.f;

    // Étape courante
    int etapeAff = _etapeCourante + 1;
    int etapeMax = (int)_etapes.size();
    string etapeStr = "Etape : " + to_string(etapeAff) + " / " + to_string(etapeMax);
    texte(etapeStr, px, py, 14, sf::Color(255, 180, 80));
    py += 22.f;

    // Mouvements de l'étape courante
    if (_etapeCourante >= 0 && _etapeCourante < (int)_etapes.size()) {
        texte("Mouvements :", px, py, 13, sf::Color(150, 200, 255));
        py += 18.f;
        for (const auto& mv : _etapes[_etapeCourante]) {
            string ligne = "  f" + to_string(mv.fourmiId)
                         + " -> " + mv.salleDest;
            texte(ligne, px, py, 12); py += 15.f;
            if (py > _hauteur - 120.f) break; // débordement
        }
    }

    py = _hauteur - 110.f;

    // Contrôles
    texte("── Controles ──", px, py, 12, sf::Color(120, 120, 150)); py += 18.f;
    texte("ESPACE / → : etape suiv.", px, py, 11, sf::Color(160,160,160)); py += 15.f;
    texte("←         : etape prec.", px, py, 11, sf::Color(160,160,160)); py += 15.f;
    texte("R         : reinitialiser",  px, py, 11, sf::Color(160,160,160)); py += 15.f;
    texte("↑ / ↓    : fourmiliere",    px, py, 11, sf::Color(160,160,160)); py += 15.f;
    texte("ESC       : quitter",        px, py, 11, sf::Color(160,160,160));
}

// ============================================================
//  Nom affiché pour une fourmilière
// ============================================================
string Visualiseur::nomFourmiliere(int index) const {
    static const vector<string> noms = {
        "Fourmiliere 0","Fourmiliere 1","Fourmiliere 2",
        "Fourmiliere 3","Fourmiliere 4","Fourmiliere 5"
    };
    if (index >= 0 && index < (int)noms.size()) return noms[index];
    return "Fourmiliere " + to_string(index);
}