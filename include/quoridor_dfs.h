#include "util.h"

///DFS pathfinding
void addEdge(struct Graph* graph, int src, int dest);
int DFS(struct Graph* graph, int block,board *b,int target);
struct stacknode* createstacknode(int v);
struct Graph* createGraph(int blocks);
int illegal_wall_check(board* b,color player);
void free_list_DFS(struct stacknode* head);
void printList(struct stacknode* temp,int* visited);
point ntp(int size,int k);
int ptn(point p,int size );