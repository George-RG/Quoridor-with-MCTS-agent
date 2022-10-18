#include "quoridor_structs.h"
#include "util.h"
#include "quoridor_wallcheck.h"
#include "quoridor_dfs.h"

#ifndef BFS_H
#define BFS_H

//BFS pathfinding
int BFS(struct Graph* graph, int startBlock,int target,board *b);
struct queue* createQueue();
int isEmpty(struct queue* q);
void enqueue(struct queue* q, int value);
int dequeue(struct queue* q);
void printQueue(struct queue* q);
int find_min_steps(board* b,color player,point start,int target);
void find_shortest_or_longest_path (board *b,color cur_agent,distance distance,point* next_moves);

#endif // BFS_H