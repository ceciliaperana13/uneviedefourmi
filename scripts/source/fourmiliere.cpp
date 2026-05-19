#include "../include/fourmiliere.hpp"
#include <iostream>

// Initialise le nombre de fourmis à zéro et crée les salles obligatoires
Fourmiliere::Fourmiliere() : nbFourmis(0) {
    initialiserSallesSpeciales();
}

// Libère toutes les salles et fourmis allouées dynamiquement
Fourmiliere::~Fourmiliere() {
    for (auto& [nom, salle] : salles) delete salle;
    for (Fourmi* f : fourmis)         delete f;
}

// Délègue la lecture au LecteurFichierTexte, puis construit le graphe
bool Fourmiliere::chargerDepuisFichier(const string& chemin) {
    DonneesFourmiliere donnees = LecteurFichierTexte::lire(chemin);

    if (donnees.nbFourmis == 0) return false;

    construireDepuisDonnees(donnees);
    return true;
}

// Applique les données brutes pour remplir salles, tunnels et fourmis
void Fourmiliere::construireDepuisDonnees(const DonneesFourmiliere& donnees) {
    nbFourmis = donnees.nbFourmis;

    for (const auto& [nom, capacite] : donnees.salles)
        ajouterSalle(nom, capacite);

    for (const auto& [nomA, nomB] : donnees.tunnels)
        ajouterTunnel(nomA, nomB);

    initialiserFourmis();
}

// Crée le vestibule (Sv) et le dortoir (Sd) avec une capacité illimitée
void Fourmiliere::initialiserSallesSpeciales() {
    salles["Sv"] = new Salle("Sv", Salle::CAPACITE_ILLIMITEE);
    salles["Sd"] = new Salle("Sd", Salle::CAPACITE_ILLIMITEE);
}

// Enregistre une salle dans le graphe si elle n'existe pas déjà
void Fourmiliere::ajouterSalle(const string& nom, int capacite) {
    getOuCreerSalle(nom, capacite);
}

// Relie deux salles par un tunnel bidirectionnel
void Fourmiliere::ajouterTunnel(const string& nomA, const string& nomB) {
    Salle* salleA = getOuCreerSalle(nomA);
    Salle* salleB = getOuCreerSalle(nomB);
    salleA->ajouterVoisin(salleB);
    salleB->ajouterVoisin(salleA);
}

// Crée une fourmi par numéro (1 à nbFourmis), toutes placées dans le vestibule
void Fourmiliere::initialiserFourmis() {
    Salle* vestibule = getVestibule();
    for (int i = 1; i <= nbFourmis; i++)
        fourmis.push_back(new Fourmi(i, vestibule));
}

// Retourne la salle existante ou en crée une nouvelle avec la capacité donnée
Salle* Fourmiliere::getOuCreerSalle(const string& nom, int capacite) {
    if (salles.count(nom) == 0)
        salles[nom] = new Salle(nom, capacite);
    return salles[nom];
}

// Affiche la structure de la fourmilière : salles, capacités et voisins
void Fourmiliere::afficher() const {
    cout << "Fourmiliere : " << nbFourmis << " fourmis, "
              << salles.size() << " salles" << endl;

    for (const auto& [nom, salle] : salles) {
        cout << "  " << nom;

        if (salle->getCapacite() == Salle::CAPACITE_ILLIMITEE)
            cout << " [illimitee]";
        else
            cout << " [cap=" << salle->getCapacite() << "]";

        // Liste des salles accessibles depuis celle-ci
        cout << " -> ";
        for (const Salle* v : salle->getVoisins())
            cout << v->getNom() << " ";

        cout << endl;
    }
}

// Retourne le pointeur vers la salle demandée, ou nullptr si elle n'existe pas
Salle* Fourmiliere::getSalle(const string& nom) const {
    auto it = salles.find(nom);
    return (it != salles.end()) ? it->second : nullptr;
}

Salle* Fourmiliere::getVestibule() const { return getSalle("Sv"); }
Salle* Fourmiliere::getDortoir()   const { return getSalle("Sd"); }
int    Fourmiliere::getNbFourmis() const { return nbFourmis; }

const map<string, Salle*>& Fourmiliere::getSalles()  const { return salles; }
const vector<Fourmi*>&          Fourmiliere::getFourmis() const { return fourmis; }