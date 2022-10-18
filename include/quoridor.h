#include "util.h"
#include "dijkstra_s.h"
#include "quoridor_structs.h"
#include "MCTS.h"

#ifndef QUORIDOR_H
#define QUORIDOR_H

//Game Commands
void name(void);//token 1
void known_command (char *buff);//token 2
void list_commands(void);//token 3
void quit_free(board *b,listptr *adr);//token 4
void boardsize(char* buff,board* b);//token 5
void clear_board(board *b,listptr* adr,int* move_num);//token 6
void walls(char *buff,player *p1,player *p2);//token 7
void playmove(char *buff,board* b,listptr* adr,int *move_num);//token 8
void playwall(board* b ,char* buff,listptr* adr,int* move_num);//token 9
int undo(listptr *adr,board *b,char *buff, int* move_num);//token 11
color winner(board b); //token 12
void showboard(board* b);//token 13


//TEST
void genmove(board *b, char* buff,int *move_num,listptr* adr);

#endif // QUORIDOR_H