#include "quoridor_wallcheck.h"

#ifndef DIJKSTRA_S_H
#define DIJKSTRA_S_H

int** create2DIntArray(int rows,int cols,int value);
point FindShortestorLongestPath(board *b, color agent,distance dist);

#endif // DIJKSTRA_S_H