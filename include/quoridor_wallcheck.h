#include "quoridor_structs.h"
#include "util.h"
#include "quoridor_dfs.h"


#ifndef QUORIDOR_WALLCHECK_H
#define QUORIDOR_WALLCHECK_H

int touching_wall(board* b,point p,orientation orient);
int legal_wall(board *b,point p,orientation orient,bool no_place);
int illegal_wall_check(board* b,color player);

#endif // QUORIDOR_WALLCHECK_H