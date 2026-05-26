# Une Vie de Fourmi — Simulateur de fourmilière

## Présentation du projet

### Contexte

Une fourmilière est un graphe de **salles** reliées par des **tunnels**. Chaque salle possède une capacité maximale d'occupation (sauf le vestibule `Sv` et le dortoir `Sd`, qui sont illimités). Un nombre défini de fourmis doit traverser ce graphe de `Sv` jusqu'à `Sd` en un **nombre minimal d'étapes**.

### Problématique

> Comment faire circuler *N* fourmis de l'entrée au dortoir le plus rapidement possible, en respectant les contraintes de capacité de chaque salle, sans provoquer de blocage (deadlock) ?

Le défi est triple :

- **Trouver** tous les chemins valides sans créer de cycles ni de croisements bloquants.
- **Répartir** les fourmis intelligemment entre ces chemins pour minimiser le nombre total d'étapes.
- **Gérer** les goulots d'étranglement : une salle à capacité 1 au milieu d'un chemin très fréquenté peut tout paralyser.

---

## Fonctionnalités

### Mode terminal

- Chargement d'une fourmilière depuis un fichier `.txt` (graphe + capacités + nombre de fourmis)
- Affichage des chemins trouvés entre `Sv` et `Sd` par le DFS
- Affichage de l'assignation de chaque fourmi à son chemin
- Simulation pas à pas : chaque étape affiche les mouvements (`fX-SY`)
- Détection et arrêt propre en cas de deadlock
- Mode Dijkstra alternatif : calcule le chemin le plus court et simule toutes les fourmis dessus

### Mode graphique (SFML)

- Visualisation du graphe : salles en cercles, tunnels en lignes, capacités affichées
- Animation des fourmis en temps réel, colorées par identifiant
- Navigation étape par étape (avancer / reculer)
- Basculement entre le mode **DFS + Round Robin** et le mode **Dijkstra**
- Panneau d'informations : algorithme actif, étape courante, durée CPU du DFS (µs), nombre total de tours
- Sélection de la fourmilière au lancement via menu interactif

---

## Réalisation du simulateur

### 1. Établissement des chemins par Deep First Search

L'algorithme `AlgoDeepFirst` explore le graphe en profondeur depuis `Sv` vers `Sd`.

Pour éviter les deadlocks circulaires — par exemple deux fourmis qui se bloquent mutuellement dans des salles à capacité 1 — une contrainte de **progression monotone** est appliquée : depuis toute salle autre que `Sv`, seuls les voisins **strictement plus proches** de `Sd` sont explorés (distance BFS préalculée depuis `Sd`).

`Sv` bénéficie d'une exception : tous ses voisins sont explorés, y compris ceux à égale distance, afin de ne pas amputer des branches entières dès le départ.

Les chemins obtenus sont triés par longueur croissante : les fourmis assignées aux chemins courts avancent plus vite, ce qui maximise le débit global en pipeline.

### 2. Répartition des fourmis en Round Robin

Une fois les chemins connus, les fourmis sont assignées une à une. À chaque assignation, on choisit le chemin pour lequel l'ajout de cette fourmi produit le **temps estimé minimal** :

```
temps_estimé = nb_tunnels + ceil(nb_fourmis_sur_ce_chemin / débit)
```

Cette formule modélise un pipeline : le premier terme est la latence (longueur du chemin), le second est le temps pour écouler toutes les fourmis compte tenu du débit. En pratique, cela revient à un round robin pondéré par la longueur des chemins.

### 3. Optimisation en fonction des capacités des salles

Le **débit** d'un chemin est le minimum des capacités des salles intermédiaires (hors `Sv` et `Sd`). Une salle à capacité 2 au milieu d'un chemin ne peut pas laisser passer plus de 2 fourmis par étape, peu importe la largeur des salles autour.

Ce débit est intégré dans le calcul de `temps_estimé` lors de l'assignation : un chemin court mais avec un goulot d'étranglement étroit peut se révéler moins efficace qu'un chemin plus long à débit supérieur. La répartition s'adapte automatiquement.

### 4. Gestion des bottlenecks par pipeline immédiat

L'exécution de chaque étape suit un **ordre de priorité** : les fourmis les plus avancées (position la plus haute sur leur chemin) se déplacent en premier. Ainsi, quand une fourmi essaie d'entrer dans une salle, la fourmi qui l'occupait vient de la libérer dans la même étape.

Ce mécanisme de *planification et commit immédiat* évite de sur-bloquer les salles à capacité limitée et permet à la colonne de fourmis de s'écouler de façon continue, comme un pipeline. Le deadlock est détecté en fin d'étape si aucun mouvement n'a été possible.

---

## Installation et compilation

### Prérequis

| Outil | Version recommandée |
|---|---|
| Compilateur | GCC / G++ ≥ 11 (MinGW sur Windows) |
| Standard C++ | C++17 |
| SFML | 2.6.x (pour le mode graphique uniquement) |

### Compilation — Mode terminal uniquement

```bash
cd scripts/source

gcc main.cpp salle.cpp fourmi.cpp fourmiliere.cpp \
    lecteur_fichier_texte.cpp algo_deep_first.cpp simulateur.cpp \
    -I../include -o uneviedefourmi -Wall -Wextra -lstdc++
```

### Compilation — Mode graphique (SFML)

**Linux / macOS** (après `sudo apt install libsfml-dev` ou `brew install sfml`) :

```bash
cd scripts/source

g++ -std=c++17 -O2 \
    main.cpp menu_principal.cpp Visualiseur.cpp Dijkstra.cpp \
    fourmiliere.cpp fourmi.cpp salle.cpp lecteur_fichier_texte.cpp \
    simulateur.cpp algo_deep_first.cpp \
    -I../include \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -o uneviedefourmi
```

**Windows (MinGW)** — adapter les chemins selon votre installation SFML :

```bash
g++ -std=c++17 -O2 \
    main.cpp menu_principal.cpp Visualiseur.cpp Dijkstra.cpp \
    fourmiliere.cpp fourmi.cpp salle.cpp lecteur_fichier_texte.cpp \
    simulateur.cpp algo_deep_first.cpp \
    -I../include \
    -I C:/SFML-2.6.1/include \
    -L C:/SFML-2.6.1/lib \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -o fourmiliere_viz.exe
```

> Les DLL SFML (`sfml-graphics-2.dll`, `sfml-window-2.dll`, `sfml-system-2.dll`) doivent se trouver dans le même dossier que l'exécutable sur Windows.

### Lancement

```bash
./uneviedefourmi        # Linux / macOS
fourmiliere_viz.exe     # Windows
```

Le menu principal propose : simulation terminal (DFS), Dijkstra, ou visualiseur graphique. Les fourmilières disponibles dans `fourmilieres/` sont listées automatiquement.

### Format d'une fourmilière

```
f=20          # nombre de fourmis
S1 { 3 }      # salle S1, capacité 3
S2            # salle S2, capacité par défaut (1)
Sv - S1       # tunnel entre vestibule et S1
S1 - S2
S2 - Sd       # tunnel vers le dortoir
```

---

## Difficultés rencontrées

### Algorithme — Deadlocks et cycles

Trouver *tous* les chemins simples dans un graphe non orienté sans tomber dans des cycles infinis a nécessité plusieurs itérations. La solution retenue — filtrage par distance BFS avec exception sur `Sv` — est le résultat de nombreux cas de tests où des fourmis se bloquaient mutuellement sur des mouvements latéraux (deux salles à égale distance de `Sd`).

### Simulation — Pipeline et gestion des capacités

La tentation initiale de séparer planification et commit en deux passes globales créait des incohérences : une fourmi réservait une place libérée par une autre qui n'avait pas encore effectivement bougé. Le passage à un commit immédiat fourmi par fourmi (les plus avancées d'abord) a résolu ces fantômes de capacité.

### Installation SFML sur Windows

L'installation de SFML avec MinGW sur Windows a posé plusieurs problèmes :

- **Incompatibilité de compilateur** : SFML distribue des binaires précompilés liés à une version précise de MinGW (runtime `libgcc`). Utiliser une version de MinGW différente provoque des erreurs de linkage ou des crashs au lancement. Il a fallu télécharger la version SFML correspondant exactement à la version de GCC utilisée.
- **DLL manquantes** : même compilé avec succès, l'exécutable ne se lance pas si les `.dll` SFML ne sont pas copiées à côté de l'`.exe`.
- **Chemins avec espaces** : les chemins Windows contenant des espaces dans `-I` ou `-L` nécessitent des guillemets, ce que certaines configurations MinGW gèrent mal.
- **Police introuvable** : SFML ne bundle pas de police par défaut ; `sf::Font::loadFromFile` échoue silencieusement si le chemin est relatif et que le répertoire de travail n'est pas celui de l'exécutable.

---

## Améliorations futures

- **Optimisation dynamique** : réassigner les fourmis en cours de simulation si un chemin se révèle plus rapide qu'estimé, en tenant compte de l'état réel des salles.
- **Support des fourmis avec poids** : certaines espèces transportent des charges ; modéliser un coût de déplacement variable selon la fourmi.
- **Éditeur graphique** de fourmilières directement dans le visualiseur SFML (glisser-déposer des salles, tracer des tunnels).
- **Export des résultats** : générer un fichier de log horodaté avec le détail de chaque étape et les métriques (nombre de tours, débit moyen, taux d'occupation des salles).
- **Parallelisation** : pour les très grandes fourmilières, explorer un calcul multi-thread du DFS.
- **Tests unitaires** : mettre en place un framework de tests (Catch2 ou Google Test) pour valider chaque composant indépendamment.

---

## Auteurs

- **Cecilia Perana**
- **Yannis Sandoval**
- **Nelson Grac-Aubert**
