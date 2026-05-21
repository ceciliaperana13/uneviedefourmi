#include "../include/Visualiseur.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

static const sf::Color PALETTE[] = {
    {220,80,80},{80,180,80},{80,120,220},
    {220,160,40},{160,80,200},{40,180,200},
    {220,120,60},{100,200,120},{180,80,140},{80,140,100}
};
static constexpr int NB_COULEURS = 10;

// ============================================================
//  Constructeur — SFML 2
// ============================================================
Visualiseur::Visualiseur(unsigned int largeur, unsigned int hauteur)
    : _fenetre(sf::VideoMode(largeur, hauteur), "Fourmiliere - Dijkstra",
               sf::Style::Titlebar | sf::Style::Close),
      _largeur(largeur), _hauteur(hauteur),
      _fm(nullptr), _etapeCourante(-1), _indexFm(0)
{
    _fenetre.setFramerateLimit(60);

    // Polices Windows
    vector<string> polices = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
    };
    for (const auto& p : polices)
        if (_police.loadFromFile(p)) break;   // SFML 2 : loadFromFile
}

// ============================================================
//  run
// ============================================================
void Visualiseur::run(const vector<string>& fichiers) {
    _fichiers = fichiers;
    _indexFm  = 0;
    chargerFourmiliere(_fichiers[0]);

    while (_fenetre.isOpen()) {
        sf::Event ev;
        while (_fenetre.pollEvent(ev)) {           // SFML 2 : pollEvent(ev)
            if (ev.type == sf::Event::Closed)
                _fenetre.close();

            if (ev.type == sf::Event::KeyPressed) {
                switch (ev.key.code) {
                    case sf::Keyboard::Right:
                    case sf::Keyboard::Space:
                        avancerEtape(); break;
                    case sf::Keyboard::Left:
                        reculerEtape(); break;
                    case sf::Keyboard::R:
                        reinitialiser(); break;
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
        if (_fm) {
            dessinerTunnels();
            dessinerSalles();
            dessinerFourmis();
        }
        dessinerPanneau();
        _fenetre.display();
    }

    delete _fm;
}

// ============================================================
//  Chargement
// ============================================================
void Visualiseur::chargerFourmiliere(const string& chemin) {
    delete _fm;
    _fm = new Fourmiliere();
    if (!_fm->chargerDepuisFichier(chemin)) {
        cerr << "Impossible de charger : " << chemin << "\n";
        delete _fm; _fm = nullptr; return;
    }
    AlgorithmeDijkstra algo(_fm);
    _res = algo.executer();
    calculerPositions();
    initialiserEtapes();
    reinitialiser();
}

// ============================================================
//  Positions
// ============================================================
void Visualiseur::calculerPositions() {
    _positions.clear();
    float zoneW = _largeur * 0.72f;
    float zoneH = _hauteur * 0.85f;
    float offX  = 30.f;
    float offY  = (_hauteur - zoneH) / 2.f;

    _positions["Sv"] = { offX + 60.f,         offY + zoneH / 2.f };
    _positions["Sd"] = { offX + zoneW - 60.f, offY + zoneH / 2.f };

    vector<string> autres;
    for (const auto& kv : _fm->getSalles())
        if (kv.first != "Sv" && kv.first != "Sd")
            autres.push_back(kv.first);

    int n = (int)autres.size();
    if (n == 0) return;
    float cx = offX + zoneW / 2.f, cy = offY + zoneH / 2.f;
    float rx = zoneW * 0.32f,      ry = zoneH * 0.38f;
    for (int i = 0; i < n; i++) {
        float a = (2.f * 3.14159f * i) / n - 3.14159f / 2.f;
        _positions[autres[i]] = { cx + rx * cosf(a), cy + ry * sinf(a) };
    }
}

// ============================================================
//  Étapes
// ============================================================
void Visualiseur::initialiserEtapes() {
    _etapes.clear();
    if (_res.chemin.empty()) return;

    vector<string> chemin;
    for (Salle* s : _res.chemin) chemin.push_back(s->getNom());

    int nbF      = _fm->getNbFourmis();
    int longu    = (int)chemin.size();
    int nbEtapes = (longu - 1) + (nbF - 1);
    _etapes.resize(nbEtapes);

    for (int f = 0; f < nbF; f++)
        for (int step = 0; step < longu - 1; step++) {
            int idx = f + step;
            if (idx < nbEtapes)
                _etapes[idx].push_back({ f + 1, chemin[step + 1] });
        }
}

void Visualiseur::reinitialiser() {
    _etapeCourante = -1;
    _posFourmis.clear();
    if (!_fm) return;
    for (int i = 1; i <= _fm->getNbFourmis(); i++)
        _posFourmis[i] = "Sv";
}

void Visualiseur::avancerEtape() {
    int mx = (int)_etapes.size() - 1;
    if (_etapeCourante >= mx) return;
    _etapeCourante++;
    for (const auto& mv : _etapes[_etapeCourante])
        _posFourmis[mv.fourmiId] = mv.salleDest;
}

void Visualiseur::reculerEtape() {
    if (_etapeCourante < 0) return;
    _etapeCourante--;
    _posFourmis.clear();
    if (!_fm) return;
    for (int i = 1; i <= _fm->getNbFourmis(); i++) _posFourmis[i] = "Sv";
    for (int e = 0; e <= _etapeCourante; e++)
        for (const auto& mv : _etapes[e])
            _posFourmis[mv.fourmiId] = mv.salleDest;
}

sf::Vector2f Visualiseur::positionSalle(const string& nom) const {
    auto it = _positions.find(nom);
    return it != _positions.end() ? sf::Vector2f(it->second.x, it->second.y) : sf::Vector2f(0,0);
}

sf::Color Visualiseur::couleurFourmi(int id) const {
    return PALETTE[(id - 1) % NB_COULEURS];
}

// ============================================================
//  Tunnels
// ============================================================
void Visualiseur::dessinerTunnels() {
    for (const auto& kv : _fm->getSalles()) {
        const string& nom   = kv.first;
        const Salle*  salle = kv.second;
        sf::Vector2f pA = positionSalle(nom);

        for (const Salle* voisin : salle->getVoisins()) {
            sf::Vector2f pB = positionSalle(voisin->getNom());

            sf::Color col(80, 80, 100);
            for (int i = 0; i + 1 < (int)_res.chemin.size(); i++) {
                bool ab = (_res.chemin[i]->getNom() == nom &&
                           _res.chemin[i+1]->getNom() == voisin->getNom());
                bool ba = (_res.chemin[i]->getNom() == voisin->getNom() &&
                           _res.chemin[i+1]->getNom() == nom);
                if (ab || ba) { col = sf::Color(255,210,50); break; }
            }

            // SFML 2 : sf::Vertex(position, color)
            sf::Vertex ligne[] = {
                sf::Vertex(pA, col),
                sf::Vertex(pB, col)
            };
            _fenetre.draw(ligne, 2, sf::Lines);
        }
    }
}

// ============================================================
//  Salles
// ============================================================
void Visualiseur::dessinerSalles() {
    const float R = 22.f;
    for (const auto& kv : _fm->getSalles()) {
        const string& nom   = kv.first;
        const Salle*  salle = kv.second;
        sf::Vector2f pos = positionSalle(nom);

        bool surChemin = false;
        for (Salle* s : _res.chemin)
            if (s->getNom() == nom) { surChemin = true; break; }

        sf::CircleShape c(R);
        c.setOrigin(R, R);              // SFML 2 : setOrigin(x, y)
        c.setPosition(pos);
        if      (nom == "Sv")  c.setFillColor(sf::Color(60,160,100));
        else if (nom == "Sd")  c.setFillColor(sf::Color(60,100,200));
        else if (surChemin)    c.setFillColor(sf::Color(80,80,50));
        else                   c.setFillColor(sf::Color(60,60,80));
        c.setOutlineThickness(2.f);
        c.setOutlineColor(surChemin ? sf::Color(255,210,50) : sf::Color(120,120,150));
        _fenetre.draw(c);

        // SFML 2 : sf::Text constructeur vide puis setFont/setString/setCharacterSize
        sf::Text t;
        t.setFont(_police);
        t.setString(nom);
        t.setCharacterSize(13);
        t.setFillColor(sf::Color::White);
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        t.setPosition(pos.x, pos.y - 4.f);
        _fenetre.draw(t);

        if (salle->getCapacite() != Salle::CAPACITE_ILLIMITEE) {
            sf::Text cap;
            cap.setFont(_police);
            cap.setString("c=" + to_string(salle->getCapacite()));
            cap.setCharacterSize(10);
            cap.setFillColor(sf::Color(180,180,180));
            sf::FloatRect cb = cap.getLocalBounds();
            cap.setOrigin(cb.left + cb.width/2.f, 0.f);
            cap.setPosition(pos.x, pos.y + R + 2.f);
            _fenetre.draw(cap);
        }
    }
}

// ============================================================
//  Fourmis
// ============================================================
void Visualiseur::dessinerFourmis() {
    map<string, vector<int>> parSalle;
    for (const auto& kv : _posFourmis)
        parSalle[kv.second].push_back(kv.first);

    const float RF = 6.f, OFF = 14.f;
    for (const auto& kv : parSalle) {
        const string&      salle = kv.first;
        const vector<int>& ids   = kv.second;
        sf::Vector2f centre = positionSalle(salle);
        int n = (int)ids.size();

        for (int i = 0; i < n; i++) {
            float a  = (n == 1) ? 0.f : (2.f * 3.14159f * i) / n;
            float ox = (n == 1) ? 0.f : OFF * cosf(a);
            float oy = (n == 1) ? 0.f : OFF * sinf(a);

            sf::CircleShape pt(RF);
            pt.setOrigin(RF, RF);
            pt.setPosition(centre.x + ox, centre.y + oy);
            pt.setFillColor(couleurFourmi(ids[i]));
            pt.setOutlineThickness(1.f);
            pt.setOutlineColor(sf::Color(255,255,255,80));
            _fenetre.draw(pt);

            if (_fm->getNbFourmis() <= 20) {
                sf::Text t;
                t.setFont(_police);
                t.setString(to_string(ids[i]));
                t.setCharacterSize(8);
                t.setFillColor(sf::Color::White);
                sf::FloatRect b = t.getLocalBounds();
                t.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
                t.setPosition(centre.x + ox, centre.y + oy - 1.f);
                _fenetre.draw(t);
            }
        }
    }
}

// ============================================================
//  Panneau
// ============================================================
void Visualiseur::dessinerPanneau() {
    float px = _largeur * 0.74f;
    float py = 20.f;

    sf::RectangleShape fond(sf::Vector2f(_largeur * 0.24f, _hauteur - 40.f));
    fond.setPosition(px - 10.f, 10.f);
    fond.setFillColor(sf::Color(20,20,35));
    fond.setOutlineThickness(1.f);
    fond.setOutlineColor(sf::Color(80,80,100));
    _fenetre.draw(fond);

    auto txt = [&](const string& s, float x, float y,
                   unsigned int sz = 14,
                   sf::Color col = sf::Color::White) {
        sf::Text t;
        t.setFont(_police);
        t.setString(s);
        t.setCharacterSize(sz);
        t.setFillColor(col);
        t.setPosition(x, y);
        _fenetre.draw(t);
    };

    txt(nomFourmiliere(_indexFm), px, py, 16, sf::Color(255,210,50)); py += 26.f;

    if (_fm) {
        txt("Fourmis : " + to_string(_fm->getNbFourmis()), px, py, 13,
            sf::Color(180,220,180)); py += 20.f;
    }

    if (!_res.chemin.empty()) {
        txt("Chemin optimal :", px, py, 13, sf::Color(150,200,255)); py += 18.f;
        string ch;
        for (int i = 0; i < (int)_res.chemin.size(); i++) {
            ch += _res.chemin[i]->getNom();
            if (i < (int)_res.chemin.size()-1) ch += "->";
            if ((i+1) % 4 == 0 && i < (int)_res.chemin.size()-1) {
                txt("  "+ch, px, py, 12); py += 16.f; ch = "";
            }
        }
        if (!ch.empty()) { txt("  "+ch, px, py, 12); py += 16.f; }
    } else {
        txt("Aucun chemin", px, py, 13, sf::Color(255,100,100)); py += 18.f;
    }

    py += 8.f;
    int eAff = _etapeCourante + 1;
    int eMax = (int)_etapes.size();
    txt("Etape : "+to_string(eAff)+" / "+to_string(eMax),
        px, py, 14, sf::Color(255,180,80)); py += 22.f;

    if (_etapeCourante >= 0 && _etapeCourante < (int)_etapes.size()) {
        txt("Mouvements :", px, py, 13, sf::Color(150,200,255)); py += 18.f;
        for (const auto& mv : _etapes[_etapeCourante]) {
            txt("  f"+to_string(mv.fourmiId)+" -> "+mv.salleDest, px, py, 12);
            py += 15.f;
            if (py > _hauteur - 120.f) break;
        }
    }

    py = _hauteur - 110.f;
    txt("── Controles ──",          px, py, 12, sf::Color(120,120,150)); py+=18.f;
    txt("ESPACE/-> : etape suiv.",  px, py, 11, sf::Color(160,160,160)); py+=15.f;
    txt("<-        : etape prec.",  px, py, 11, sf::Color(160,160,160)); py+=15.f;
    txt("R         : reinit.",      px, py, 11, sf::Color(160,160,160)); py+=15.f;
    txt("haut/bas  : fourmiliere",  px, py, 11, sf::Color(160,160,160)); py+=15.f;
    txt("ESC       : quitter",      px, py, 11, sf::Color(160,160,160));
}

string Visualiseur::nomFourmiliere(int i) const {
    static const vector<string> noms = {
        "Fourmiliere 0","Fourmiliere 1","Fourmiliere 2",
        "Fourmiliere 3","Fourmiliere 4","Fourmiliere 5"
    };
    return (i >= 0 && i < (int)noms.size()) ? noms[i] : "Fourmiliere "+to_string(i);
}