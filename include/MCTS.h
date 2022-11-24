#include "quoridor_structs.h"
#include "util.h"
#include "quoridor_wallcheck.h"
#include "BFS.h"
#include "dijkstra_s.h"
#include "List.h"

#ifndef MCTS_H
#define MCTS_H

//MCTS
double MCTS(board *original_board,color player,listptr* adr, int* move_num);
tree_node* findPromisingNode(tree_node *root,board* b);
tree_node* FindBestChild(tree_node node);
tree_node* FindBestChild_PASSES(tree_node node);
tree_node* GetNth(int n, tlptr list);
double GetUCT(int parent_visits,tree_node node);
void expand_node(tree_node *node , board *b);
void PerformMove(tree_node* node,board *b);
void add_child(tree_node *node,color player,move move,point p,orientation orient);
void* SimulateRollout(void* args);
bool Heuristic_Move(color cur_agent,board *b,bool no_wall,int turns);
point find_probable_next_wall(board *b,color cur_agent,orientation* orient_return);
point find_all_next_wall(board *b,color cur_agent,orientation* orient_return);
void backPropagation(tree_node *node, double result,color player,int passes);
void free_childs(tree_node* root);
int evaluate(board b);

tree_node* SelectBestChild(tree_node node);
bool Semirandom_Move(color cur_agent,board *b,bool no_wall,int turns);
#endif // MCTS_H