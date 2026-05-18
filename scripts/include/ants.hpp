#pragma once

#include <vector>

using namespace std;

// FOURMILIERES : MATRICES D'ADJACENCE

const vector<vector<int>> fourmiliereZero = {{0, 1, 1, 0},
                                             {1, 0, 0, 1},
                                             {1, 0, 0, 1},
                                             {0, 1, 1, 0}};

const vector<vector<int>> fourmiliereUn = {{0, 1, 0, 1},
                                          {1, 0, 1, 0},
                                          {0, 1, 0, 1},
                                          {1, 0, 1, 0}};

const vector<vector<int>> fourmiliereDeux;
const vector<vector<int>> fourmiliereTrois;
const vector<vector<int>> fourmiliereQuatre;
const vector<vector<int>> fourmiliereCinq;
