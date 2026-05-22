#include "../include/Visualiseur.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <queue>

using namespace std;
using namespace std::chrono;

static const sf::Color PALETTE[] = {
    {220,80,80},{80,180,80},{80,120,220},
    {220,160,40},{160,80,200},{40,180,200},
    {220,120,60},{100,200,120},{180,80,140},{80,140,100}
};
static constexpr int NB_COULEURS = 10;

// ============================================================
//  Constructeur
// ============================================================
Visualiseur::Visualiseur(unsigned int largeur, unsigned int hauteur)
    : _fenetre(sf::VideoMode(largeur, hauteur), "Fourmiliere - DFS + Dijkstra",
               sf::Style::Titlebar | sf::Style::Close),
      _largeur(largeur), _hauteur(hauteur),
      _fm(nullptr), _etapeCourante(-1), _indexFm(0),
      _mode(ModeAnimation::DIJKSTRA)
{
    _fenetre.setFramerateLimit(60);
    for (const auto& p : {
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
            "C:/Windows/Fonts/segoeui.ttf" })
        if (_police.loadFromFile(p)) break;
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
        while (_fenetre.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) _fenetre.close();
            if (ev.type == sf::Event::KeyPressed) {
                switch (ev.key.code) {
                    case sf::Keyboard::Right:
                    case sf::Keyboard::Space:  avancerEtape();   break;
                    case sf::Keyboard::Left:   reculerEtape();   break;
                    case sf::Keyboard::R:      reinitialiser();  break;
                    case sf::Keyboard::Tab:    basculerMode();   break;
                    case sf::Keyboard::Up:
                    case sf::Keyboard::PageUp:
                        _indexFm = (_indexFm + 1) % (int)_fichiers.size();
                        chargerFourmiliere(_fichiers[_indexFm]); break;
                    case sf::Keyboard::Down:
                    case sf::Keyboard::PageDown:
                        _indexFm = (_indexFm - 1 + (int)_fichiers.size()) % (int)_fichiers.size();
                        chargerFourmiliere(_fichiers[_indexFm]); break;
                    case sf::Keyboard::Escape: _fenetre.close(); break;
                    default: break;
                }
            }
        }
        _fenetre.clear(sf::Color(30, 30, 40));
        if (_fm) { dessinerTunnels(); dessinerSalles(); dessinerFourmis(); }
        dessinerPanneau();
        _fenetre.display();
    }
    delete _fm;
}

// ============================================================
//  chargerFourmiliere
// ============================================================
void Visualiseur::chargerFourmiliere(const string& chemin) {
    delete _fm;
    _fm = new Fourmiliere();
    if (!_fm->chargerDepuisFichier(chemin)) {
        cerr << "Impossible de charger : " << chemin << "\n";
        delete _fm; _fm = nullptr; return;
    }

    // == 1. DFS ===============================================
    {
        auto t0 = high_resolution_clock::now();
        AlgoDeepFirst dfs;
        _cheminsDFS = dfs.trouverTousLesChemins(*_fm);
        auto t1 = high_resolution_clock::now();
        _tempsDfsUs = duration_cast<microseconds>(t1 - t0).count();
    }

    // == 2. Simulateur DFS ====================================
    {
        streambuf* oldBuf = cout.rdbuf(nullptr);
        Simulateur sim(*_fm);
        sim.simuler();
        cout.rdbuf(oldBuf);
        _etapesDFS = sim.getEtapes();
    }

    // == 3. Dijkstra ==========================================
    {
        streambuf* oldBuf = cout.rdbuf(nullptr);
        AlgorithmeDijkstra algo(_fm);
        _resDijkstra = algo.executer();
        cout.rdbuf(oldBuf);
        _tempsSimSec = _resDijkstra.nbTours;
    }

    calculerPositions();
    initialiserEtapesDijkstra();
    reinitialiser();
}

// ============================================================
//  calculerPositions
// ============================================================
void Visualiseur::calculerPositions() {
    _positions.clear();
    float zoneW = _largeur * 0.72f, zoneH = _hauteur * 0.85f;
    float offX  = 30.f, offY = (_hauteur - zoneH) / 2.f;

    _positions["Sv"] = { offX + 60.f,         offY + zoneH / 2.f };
    _positions["Sd"] = { offX + zoneW - 60.f, offY + zoneH / 2.f };

    vector<string> autres;
    for (const auto& kv : _fm->getSalles())
        if (kv.first != "Sv" && kv.first != "Sd")
            autres.push_back(kv.first);

    int n = (int)autres.size(); if (!n) return;
    float cx = offX + zoneW/2.f, cy = offY + zoneH/2.f;
    float rx = zoneW*0.32f,      ry = zoneH*0.38f;
    for (int i = 0; i < n; i++) {
        float a = (2.f*3.14159f*i)/n - 3.14159f/2.f;
        _positions[autres[i]] = { cx + rx*cosf(a), cy + ry*sinf(a) };
    }
}

// ============================================================
//  initialiserEtapesDijkstra
//
//  Pipeline multi-chemins avec enchainement garanti 
 
// ============================================================
void Visualiseur::initialiserEtapesDijkstra() {
    _etapesDijkstra.clear();
    if (!_fm) return;

    const auto& salles = _fm->getSalles();
    const string nomSv = _fm->getVestibule()->getNom();
    const string nomSd = _fm->getDortoir()->getNom();
    const int N = _fm->getNbFourmis();
    if (N <= 0) return;

    // ----------------------------------------------------------
    // 1) BFS depuis Sd pour connaitre la distance de chaque
    //    salle jusqu'a Sd (sans tenir compte des capacites).
    //    Sert UNIQUEMENT a trier les fourmis : celles les plus
    //    proches de Sd (distance faible) bougent en premier.
    // ----------------------------------------------------------
    map<string, int> distToSd;
    {
        queue<string> q;
        q.push(nomSd);
        distToSd[nomSd] = 0;
        while (!q.empty()) {
            string cur = q.front(); q.pop();
            auto it = salles.find(cur);
            if (it == salles.end()) continue;
            for (const Salle* v : it->second->getVoisins()) {
                const string& vn = v->getNom();
                if (!distToSd.count(vn)) {
                    distToSd[vn] = distToSd[cur] + 1;
                    q.push(vn);
                }
            }
        }
    }

    // ----------------------------------------------------------
    // 2) Etat initial
    // ----------------------------------------------------------
    map<int, string> pos;               // fourmi id -> salle courante
    map<string, int> occ;              // occupation en temps reel
    for (const auto& kv : salles) occ[kv.first] = 0;
    for (int id = 1; id <= N; id++) pos[id] = nomSv;
    occ[nomSv] = N;

    // ----------------------------------------------------------
    // 3) nextStep : BFS depuis 'from' vers Sd en respectant occ
    //    Retourne la prochaine salle a atteindre, "" si bloque.
    //    Capture occ par reference -> voit les mises a jour
    //    faites pendant le tour en cours.
    // ----------------------------------------------------------
    auto nextStep = [&](const string& from) -> string {
        if (from == nomSd) return "";

        // BFS
        map<string, string> par;   // par[X] = salle d'ou on vient pour atteindre X
        queue<string> q;
        q.push(from);
        par[from] = "";            // sentinelle : racine du BFS
        bool found = false;

        while (!q.empty() && !found) {
            string cur = q.front(); q.pop();
            auto it = salles.find(cur);
            if (it == salles.end()) continue;

            for (const Salle* v : it->second->getVoisins()) {
                const string& vn = v->getNom();
                if (par.count(vn)) continue;   // deja visite

                // Verifier la capacite (sauf pour Sd, toujours accessible)
                if (vn != nomSd) {
                    int cap = v->getCapacite();
                    if (cap != Salle::CAPACITE_ILLIMITEE) {
                        int occupes = occ.count(vn) ? occ.at(vn) : 0;
                        if (occupes >= cap) continue;   // pleine
                    }
                }

                par[vn] = cur;
                if (vn == nomSd) { found = true; break; }
                q.push(vn);
            }
        }

        if (!found) return "";

        // Reconstruction du chemin de Sd vers 'from'
        // puis inversion pour trouver le 1er pas apres 'from'
        vector<string> path;
        for (string c = nomSd; !c.empty(); c = par.at(c))
            path.push_back(c);
        // path = [Sd, ..., from] -> inverse = [from, ..., Sd]
        reverse(path.begin(), path.end());
        // path[0] = from, path[1] = premier pas
        return (path.size() >= 2) ? path[1] : "";
    };

    // ----------------------------------------------------------
    // 4) Boucle de simulation tour par tour
    // ----------------------------------------------------------
    int arrived  = 0;
    const int maxTours = (N + (int)salles.size()) * 4 + 20;

    for (int t = 0; t < maxTours && arrived < N; t++) {

        
        vector<int> ordre;
        for (int id = 1; id <= N; id++)
            if (pos[id] != nomSd) ordre.push_back(id);

        
        
        sort(ordre.begin(), ordre.end(), [&](int a, int b) {
            int da = distToSd.count(pos[a]) ? distToSd.at(pos[a]) : 99999;
            int db = distToSd.count(pos[b]) ? distToSd.at(pos[b]) : 99999;
            return da < db;   // croissant = plus proche de Sd en premier
        });

        vector<MouvementEtape> mvs;
        bool anyMoved = false;

        for (int id : ordre) {
            // BFS dynamique : occ[] est mis a jour en temps reel
            // donc nextStep voit les places liberees dans ce tour
            string next = nextStep(pos[id]);
            if (next.empty()) continue;

            // Liberation immediate de la place source
            occ[pos[id]]--;
            occ[next]++;
            pos[id] = next;

            mvs.push_back({ id, next });
            anyMoved = true;
            if (next == nomSd) arrived++;
        }

        if (!mvs.empty()) _etapesDijkstra.push_back(mvs);
        if (!anyMoved) break;   // deadlock reel, on arrete
    }
}

// ============================================================
//  Navigation
// ============================================================
const vector<vector<MouvementEtape>>& Visualiseur::etapesActives() const {
    return (_mode == ModeAnimation::DFS) ? _etapesDFS : _etapesDijkstra;
}

void Visualiseur::reinitialiser() {
    _etapeCourante = -1;
    _posFourmis.clear();
    if (!_fm) return;
    for (int i = 1; i <= _fm->getNbFourmis(); i++) _posFourmis[i] = "Sv";
}

void Visualiseur::basculerMode() {
    _mode = (_mode == ModeAnimation::DIJKSTRA)
          ? ModeAnimation::DFS
          : ModeAnimation::DIJKSTRA;
    reinitialiser();
}

void Visualiseur::avancerEtape() {
    const auto& etapes = etapesActives();
    if (_etapeCourante >= (int)etapes.size() - 1) return;
    _etapeCourante++;
    for (const auto& mv : etapes[_etapeCourante])
        _posFourmis[mv.fourmiId] = mv.salleDest;
}

void Visualiseur::reculerEtape() {
    if (_etapeCourante < 0) return;
    _etapeCourante--;
    _posFourmis.clear();
    if (!_fm) return;
    for (int i = 1; i <= _fm->getNbFourmis(); i++) _posFourmis[i] = "Sv";
    const auto& etapes = etapesActives();
    for (int e = 0; e <= _etapeCourante; e++)
        for (const auto& mv : etapes[e])
            _posFourmis[mv.fourmiId] = mv.salleDest;
}

// ============================================================
//  Helpers
// ============================================================
sf::Vector2f Visualiseur::positionSalle(const string& nom) const {
    auto it = _positions.find(nom);
    return it != _positions.end()
        ? sf::Vector2f(it->second.x, it->second.y) : sf::Vector2f(0, 0);
}

sf::Color Visualiseur::couleurFourmi(int id) const {
    return PALETTE[(id - 1) % NB_COULEURS];
}

// ============================================================
//  Dessiner tunnels
// ============================================================
void Visualiseur::dessinerTunnels() {
    const auto& chemin = _resDijkstra.chemin;
    for (const auto& kv : _fm->getSalles()) {
        sf::Vector2f pA = positionSalle(kv.first);
        for (const Salle* v : kv.second->getVoisins()) {
            sf::Vector2f pB = positionSalle(v->getNom());
            sf::Color col(80, 80, 100);
            if (_mode == ModeAnimation::DIJKSTRA) {
                for (int i = 0; i+1 < (int)chemin.size(); i++) {
                    bool ab = chemin[i]->getNom() == kv.first    && chemin[i+1]->getNom() == v->getNom();
                    bool ba = chemin[i]->getNom() == v->getNom() && chemin[i+1]->getNom() == kv.first;
                    if (ab || ba) { col = sf::Color(255,210,50); break; }
                }
            }
            sf::Vertex ligne[] = { sf::Vertex(pA, col), sf::Vertex(pB, col) };
            _fenetre.draw(ligne, 2, sf::Lines);
        }
    }
}

// ============================================================
//  Dessiner salles
// ============================================================
void Visualiseur::dessinerSalles() {
    const auto& chemin = _resDijkstra.chemin;
    const float R = 22.f;
    for (const auto& kv : _fm->getSalles()) {
        const string& nom   = kv.first;
        const Salle*  salle = kv.second;
        sf::Vector2f  pos   = positionSalle(nom);

        bool surChemin = false;
        if (_mode == ModeAnimation::DIJKSTRA)
            for (const Salle* s : chemin) if (s->getNom() == nom) { surChemin = true; break; }

        sf::CircleShape c(R);
        c.setOrigin(R, R); c.setPosition(pos);
        if      (nom == "Sv") c.setFillColor(sf::Color(60,160,100));
        else if (nom == "Sd") c.setFillColor(sf::Color(60,100,200));
        else if (surChemin)   c.setFillColor(sf::Color(80,80,50));
        else                  c.setFillColor(sf::Color(60,60,80));
        c.setOutlineThickness(2.f);
        c.setOutlineColor(surChemin ? sf::Color(255,210,50) : sf::Color(120,120,150));
        _fenetre.draw(c);

        sf::Text t; t.setFont(_police); t.setString(nom); t.setCharacterSize(15);
        t.setFillColor(sf::Color::White);
        auto b = t.getLocalBounds();
        t.setOrigin(b.left+b.width/2.f, b.top+b.height/2.f);
        t.setPosition(pos.x, pos.y-4.f);
        _fenetre.draw(t);

        if (salle->getCapacite() != Salle::CAPACITE_ILLIMITEE) {
            sf::Text cap; cap.setFont(_police);
            cap.setString("c="+to_string(salle->getCapacite()));
            cap.setCharacterSize(12); cap.setFillColor(sf::Color(180,180,180));
            auto cb = cap.getLocalBounds();
            cap.setOrigin(cb.left+cb.width/2.f, 0.f);
            cap.setPosition(pos.x, pos.y+R+2.f);
            _fenetre.draw(cap);
        }
    }
}

// ============================================================
//  Dessiner fourmis
// ============================================================
void Visualiseur::dessinerFourmis() {
    map<string, vector<int>> parSalle;
    for (const auto& kv : _posFourmis) parSalle[kv.second].push_back(kv.first);

    const float RF=6.f, OFF=14.f;
    for (const auto& kv : parSalle) {
        sf::Vector2f centre = positionSalle(kv.first);
        int n = (int)kv.second.size();
        for (int i = 0; i < n; i++) {
            float a  = (n==1)?0.f:(2.f*3.14159f*i)/n;
            float ox = (n==1)?0.f:OFF*cosf(a);
            float oy = (n==1)?0.f:OFF*sinf(a);
            sf::CircleShape pt(RF);
            pt.setOrigin(RF,RF);
            pt.setPosition(centre.x+ox, centre.y+oy);
            pt.setFillColor(couleurFourmi(kv.second[i]));
            pt.setOutlineThickness(1.f);
            pt.setOutlineColor(sf::Color(255,255,255,80));
            _fenetre.draw(pt);
            if (_fm->getNbFourmis() <= 20) {
                sf::Text t; t.setFont(_police);
                t.setString(to_string(kv.second[i]));
                t.setCharacterSize(10); t.setFillColor(sf::Color::White);
                auto b = t.getLocalBounds();
                t.setOrigin(b.left+b.width/2.f, b.top+b.height/2.f);
                t.setPosition(centre.x+ox, centre.y+oy-1.f);
                _fenetre.draw(t);
            }
        }
    }
}

// ============================================================
//  dessinerPanneau
// ============================================================
void Visualiseur::dessinerPanneau() {
    float px = _largeur * 0.74f, py = 14.f;

    sf::RectangleShape fond(sf::Vector2f(_largeur * 0.24f, _hauteur - 28.f));
    fond.setPosition(px - 10.f, 10.f);
    fond.setFillColor(sf::Color(20, 20, 35));
    fond.setOutlineThickness(1.f); fond.setOutlineColor(sf::Color(80, 80, 100));
    _fenetre.draw(fond);

    auto txt = [&](const string& s, float x, float y,
                   unsigned int sz = 14, sf::Color col = sf::Color::White) {
        sf::Text t; t.setFont(_police); t.setString(s);
        t.setCharacterSize(sz); t.setFillColor(col); t.setPosition(x, y);
        _fenetre.draw(t);
    };
    auto sep = [&](const string& label, sf::Color col = sf::Color(100,200,255)) {
        txt(label, px, py, 14, col); py += 18.f;
    };

    txt(nomFourmiliere(_indexFm), px, py, 17, sf::Color(255,210,50)); py += 24.f;
    if (_fm) {
        txt("Fourmis : " + to_string(_fm->getNbFourmis()), px, py, 14, sf::Color(180,220,180));
        py += 20.f;
    }

    bool modeDFS = (_mode == ModeAnimation::DFS);
    txt(modeDFS ? "[ Mode : DFS ]" : "[ Mode : Dijkstra ]",
        px, py, 14, modeDFS ? sf::Color(100,220,255) : sf::Color(255,210,50));
    py += 20.f;

    py += 4.f;
    sep("== DFS (" + to_string((int)_cheminsDFS.size()) + " chemin(s)) ==");

    int affMax = min((int)_cheminsDFS.size(), 6);
    for (int ci = 0; ci < affMax; ci++) {
        const auto& ch = _cheminsDFS[ci];
        string ligne = to_string(ci+1) + ") ";
        for (int j = 0; j < (int)ch.size(); j++) {
            ligne += ch[j]->getNom();
            if (j < (int)ch.size()-1) ligne += "->";
        }
        if ((int)ligne.size() > 33) ligne = ligne.substr(0,30) + "...";
        txt("  " + ligne, px, py, 12, sf::Color(200,200,200)); py += 15.f;
    }
    if ((int)_cheminsDFS.size() > affMax) {
        txt("  (+" + to_string((int)_cheminsDFS.size()-affMax) + " autres)",
            px, py, 12, sf::Color(150,150,150));
        py += 15.f;
    }
    txt("  CPU DFS : " + to_string(_tempsDfsUs) + " us",
        px, py, 12, sf::Color(120,120,120)); py += 17.f;

    py += 4.f;
    sep("== Dijkstra (optimal) ==");

    const auto& chemin = _resDijkstra.chemin;
    if (!chemin.empty()) {
        string ch;
        for (int i = 0; i < (int)chemin.size(); i++) {
            ch += chemin[i]->getNom();
            if (i < (int)chemin.size()-1) ch += "->";
        }
        if ((int)ch.size() <= 33) {
            txt("  " + ch, px, py, 13, sf::Color(255,240,180)); py += 16.f;
        } else {
            size_t mid = ch.rfind("->", ch.size()/2 + 8);
            if (mid == string::npos) mid = 30;
            txt("  " + ch.substr(0, mid+2), px, py, 13, sf::Color(255,240,180)); py += 15.f;
            txt("  " + ch.substr(mid+2),    px, py, 13, sf::Color(255,240,180)); py += 15.f;
        }
        const string& nomD = _fm->getDortoir()->getNom();
        auto it = _resDijkstra.distances.find(nomD);
        if (it != _resDijkstra.distances.end() && it->second != INT_MAX)
            txt("  Distance : " + to_string(it->second) + " tunnel(s)",
                px, py, 13, sf::Color(180,180,180));
        py += 16.f;
        txt("  CPU Dijkstra : " + to_string(_resDijkstra.tempsUs) + " us",
            px, py, 12, sf::Color(120,120,120)); py += 17.f;
    } else {
        txt("  Aucun chemin Sv -> Sd", px, py, 13, sf::Color(255,100,100)); py += 18.f;
    }

    py += 4.f;
    sep("== Temps de Sv a Sd ==");
    txt("  " + to_string((int)_etapesDijkstra.size()) + " tour(s)",
        px, py, 16, sf::Color(80,230,130)); py += 20.f;
    txt("  1 tour de pipeline = 1 s", px, py, 12, sf::Color(120,120,120)); py += 17.f;

    py += 4.f;
    sep("== Animation ==");
    const auto& etapes = etapesActives();
    txt("Etape : " + to_string(_etapeCourante+1) + " / " + to_string((int)etapes.size()),
        px, py, 15, sf::Color(255,180,80)); py += 20.f;

    if (_etapeCourante >= 0 && _etapeCourante < (int)etapes.size()) {
        txt("Mouvements :", px, py, 14, sf::Color(150,200,255)); py += 16.f;
        for (const auto& mv : etapes[_etapeCourante]) {
            txt("  f"+to_string(mv.fourmiId)+" -> "+mv.salleDest, px, py, 13);
            py += 15.f;
            if (py > _hauteur - 130.f) break;
        }
    }

    py = _hauteur - 132.f;
    txt("== Controles ==",           px, py, 13, sf::Color(120,120,150)); py += 18.f;
    txt("TAB         : switch mode", px, py, 12, sf::Color(160,160,160)); py += 15.f;
    txt("ESPACE / -> : etape suiv.", px, py, 12, sf::Color(160,160,160)); py += 15.f;
    txt("<-          : etape prec.", px, py, 12, sf::Color(160,160,160)); py += 15.f;
    txt("R           : reinit.",     px, py, 12, sf::Color(160,160,160)); py += 15.f;
    txt("haut / bas  : fourmiliere", px, py, 12, sf::Color(160,160,160)); py += 15.f;
    txt("ESC         : quitter",     px, py, 12, sf::Color(160,160,160));
}

// ============================================================
//  nomFourmiliere
// ============================================================
string Visualiseur::nomFourmiliere(int i) const {
    return (i >= 0 && i < (int)_fichiers.size())
        ? filesystem::path(_fichiers[i]).stem().string()
        : "Fourmiliere " + to_string(i);
}