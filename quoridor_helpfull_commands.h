#include "quoridor_dfs.h"

//Helpfull commands
int get_command_id(char* buff);
void clean_input(char *buff);
point vertex_to_cordinates(char* vertex,int size);
int wall_up(point p,board *b);
int wall_down(point p,board *b);
int wall_left(point p,board *b);
int wall_right(point p,board *b);
int ptn(point p,int size );
point ntp(int size,int k);
int legal_wall(board *b,point p,orientation orient,bool no_place);
int legal_move(board *b,point p,player *pl,player *pl2);
void addto_undo(move move,color player,point p,listptr* adr,int *move_num);
void free_pointlist(pointptr list);
point remove_nth_pointlist (int n,pointptr *list);
int add_pointlist (point p,pointptr *list);
tree_node* SelectRandomChild(tree_node node);
int count_childs(tree_node node);
board boardcpy(board original_board);
void free_board(board b);
void print_childs(tlptr list);
void cords_to_vertex(point p, char* vertex,board *b);
void compare_paths(int *a,int *b,distance distance, point* next_moves);
int touching_wall(board* b,point p,orientation orient);