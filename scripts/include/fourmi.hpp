#pragma once
#include <string>
#include <sstream>

using namespace std;

class Salle;

class Fourmi {
public:
    Fourmi(int id, Salle* salleDepart);

    // Retourne faux si la salle de destination n'a pas la place
    bool seDeplacer(Salle* destination);

    bool estAuDortoir() const;

    // Appeler apres seDeplacer, en passant le nom que la fourmi a quitté 
    std::string formatDeplacement(const std::string& nomOrigine) const;

    int    getId()            const;
    Salle* getSalleActuelle() const;

private:
    int    id;
    Salle* salleActuelle;
};
