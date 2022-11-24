/**
 * @file quoridor.c 
 * @author sdi2100161,sdi2100118
 * @brief 
 * @version 0.1
 * @date 2022-01-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "quoridor.h"


// Function_declerretion

const char space[] = " \t\n"; //where to split each command's words (tokens)

char* protocol_commands[] = {
	"",
	"name",
	"known_command",
	"list_commands",
	"quit",
	"boardsize",
	"clear_board",
	"walls",
	"playmove",
	"playwall",
	"genmove",
	"undo",
	"winner",
	"showboard"
};

int main (void){

	char buff[101];  board  b; listptr undo_ptr_adr=NULL;
	b.size =0;
	color win;
	int game_move_n;
	srand(time(NULL));

	printf("Welcome to Quoridor\n");

	while ( fgets(buff, 100, stdin) )
	{	
		win = NO_PLAYER;
		if( buff[0] == '#' || buff[0] == '\n' /*|| buff[0]=='\t'*/) continue; //goto next fgets

		clean_input(buff);

		switch (get_command_id(buff)) {
			case 0:
				printf("? unknown command\n\n");
				fflush(stdout);
				break;
			case 1:
				name();
				break;
			case 2:
				known_command(buff);
				break;
			case 3:
				list_commands();
				break;	
			case 4:
				quit_free(&b,&undo_ptr_adr);
				printf("=\n\n");	
				return 0;
				break;	
			case 5:
				boardsize(buff,&b);
				break;
			case 6:
				clear_board(&b,&undo_ptr_adr,&game_move_n);
				break;	
			case 7:
				walls(buff,&b.player1,&b.player2);	
				break;
			case 8:
				playmove(buff,&b,&undo_ptr_adr,&game_move_n);
				break;	
			case 9:
				playwall(&b,buff,&undo_ptr_adr,&game_move_n);
				break;
			case 10:
				genmove(&b,buff,&game_move_n,&undo_ptr_adr);
				break;		
			case 11:
				undo(&undo_ptr_adr,&b,buff,&game_move_n);
				break;
			case 12:
				win = winner(b);
				if(win==WHITE)
					printf("= true white\n\n");
				else if(win==BLACK)
					printf("= true black\n\n");
				else
					printf("= false\n\n");
				break;
			case 13:
				showboard(&b);
				break;
			default:
				break;
		}
		fflush(stdout);
	}	
	return 0;
}

// Command_Functions

//token 1
void name(void)
{
	printf("=MCTS v4\n"); //quoridor by sdi2100118 and sdi2100161 
	fflush(stdout);
}

//token 2
void known_command (char *buff)
{

	char *token;
	token = strtok(buff, space);   
	
	token = strtok(NULL, space);
	if (token == NULL){
		printf( "? No arguments\n\n");
		fflush(stdout);
		return;
	}

	if (get_command_id(token)) printf ("=True\n\n");
	else printf ("=False\n\n"); 
	
	fflush(stdout);
}

//token 3
void list_commands(void)
{
	int i;
	printf("=\n");
	for (i=1; i<(sizeof(protocol_commands)/sizeof(char *)); i++)
		printf("%s\n",protocol_commands[i]);
	printf("\n");
	fflush(stdout);
}

//token 4
void quit_free(board *b,listptr *adr)
{
	int i;
	for(i=0;i<b->size;i++)
	{
		free(b->cells[i]);
	}	
	free(b->cells);

	if(*adr != NULL)
	{
		listptr tmp;
		while(*adr != NULL)
		{
			tmp = *adr;
			*adr = (*adr)->next;
			free(tmp);
		}
	}
}

//token 5
void boardsize(char *buff,board *b)
{
	char* token;
	int i;

	if(b->size != 0)
	{
		for(i=0;i<b->size;i++)
		{
			free(b->cells[i]);
		}
		free(b->cells);
	}

	token = strtok(buff, space);
	token = strtok(NULL, space); 

	if(token == NULL)
	{
		printf("? Usage: boarsize 'size' \n\n");
		return;
	}
	
	b->size = atoi(token);

	if(b->size==1 || b->size % 2==0)
	{
		printf("? Unacceptable size\n\n");
		return;
	}		

	b->cells=malloc(b->size*sizeof(cell*)); //Creating the 2D array of cells thet would be our board 
	if(b->cells==NULL)
	{
		printf("? Memory error\n\n");
		return;
	}
	for(i=0;i<b->size;i++)
	{
		b->cells[i]=malloc((b->size)*sizeof(cell));
		if(b->cells[i]==NULL)
		{
			printf("? Memory error\n\n");
			return;
		}
	}
	printf("=\n\n");
	fflush(stdout);
}

//token 6
void clear_board(board *b,listptr* adr,int* move_num)
{

	int i,j;

	*move_num=0;

	for (i=0;  i< b->size; i++)
		for(j=0; j< b->size; j++)
		{
			b->cells[i][j].wall = NO_WALL;
			b->cells[i][j].player = NO_PLAYER;
		}

	b->player1.player_color = WHITE;
	b->player2.player_color = BLACK;

	b->player2.position.y=0 ; b->player2.position.x= b->size /2; //Initializing the players position on the board
	b->player1.position.y=b->size -1 ; b->player1.position.x=b->size/2 ;
	
	b->cells[b->size-1][b->size/2].player = b->player1.player_color; //Initializing the playes in the cells array
	b->cells[0][b->size/2].player = b->player2.player_color;

	b->player1.available_walls = b->player2.available_walls = 10;

	if(*adr != NULL)
	{
		listptr tmp;
		while(*adr != NULL)
		{
			tmp = *adr;
			*adr = (*adr)->next;
			free(tmp);
		}
	}

	printf("=\n\n");
	fflush(stdout);
}
 
//token 7
void walls(char *buff,player *p1,player *p2)
{
	char* token;
	

	token = strtok(buff, space);
	token = strtok(NULL, space);

	if(token == NULL || atoi(token) <= 0)
	{
		printf("? Invalid number(Usage:walls 'integer>0')\n\n");
		return;	
	}

	p1->available_walls=atoi(token);
	p2->available_walls=atoi(token);
	

	printf("=\n\n");
	fflush(stdout);
}

void playmove(char *buff,board* b,listptr* adr,int *move_num)//token 8
{
    player *p1,*p2,*p;
	char* token;
	point point;
	color player;

    token=strtok(buff,space);
    token=strtok(NULL,space);

	if(token == NULL)
	{
		printf("?syntax error\n\n");
		fflush(stdout);
		return;
	}

	if(token[0]=='w' || token[0]=='W')
		player=WHITE;
	else if(token[0]== 'b' || token[0]=='B')
		player=BLACK;
	else
	{
		printf("?syntax error\n\n");
		fflush(stdout);
		return;
	}

	p1=&b->player1;
	p2=&b->player2;

	if (player == WHITE)
		p = p1;
	else
		p = p2;	

	token=strtok(NULL,space);
	if (token == NULL)
	{
        printf("? syntax error\n\n"); 
        fflush(stdout); 
        return;
    }

	point=vertex_to_cordinates(token,b->size-1);
	if(point.x==-1 || point.y==-1 || point.x>b->size-1 || point.y>b->size-1)
	{
		printf("? syntax error\n\n"); 
        fflush(stdout); 
        return;
	}

	int k;
	
	if (player == WHITE)
		k = legal_move(b,point,p1,p2);
	else
		k = legal_move(b,point,p2,p1);

	if (k==NO)
	{
		printf("? illegal move\n\n");
		fflush(stdout);
		return;
	}
	else
	{
		addto_undo(TRANSFER,player,p->position,adr,move_num);
		b->cells[p->position.y][p->position.x].player=NO_PLAYER;
		b->cells[point.y][point.x].player=p->player_color;
		p->position.x=point.x;
		p->position.y=point.y;
		printf("=\n\n");
		fflush(stdout);
	}
}

void playwall(board* b ,char* buff,listptr* adr,int* move_num)//token 9
{
	char* token;
	point p;color player;
	orientation orient;

	token=strtok(buff,space);//COMMAND

	token=strtok(NULL,space);//PLAYER
	if(token[0]=='w' || token[0]=='W')
		player=WHITE;
	else if(token[0]== 'b' || token[0]=='B')
		player=BLACK;
	else
	{
		printf("?syntax error\n\n");
		fflush(stdout);
		return;
	}

	if (player == WHITE)//Check if player has enough walls left
	{
		if(b->player1.available_walls==0)
		{
			printf("?Not enough walls\n\n");
			fflush(stdout);
			return;
		}
	}	
	else
	{
		if(b->player2.available_walls==0)
		{
			printf("?Not enough walls\n\n");
			fflush(stdout);
			return;
		}
	}

    token=strtok(NULL,space);//VERTEX
	p=vertex_to_cordinates(token,b->size-1);
	if(p.x==-1 || p.y==-1 || p.x==(b->size)-1 || p.y==b->size-1)
	{
		printf("? syntax error\n\n"); 
        fflush(stdout); 
        return;
	}

	token=strtok(NULL,space); // ORIENTATION
	if(token[0]=='h' || token[0]=='H')
		orient=HORIZONTAL;
	else if(token[0]== 'v' || token[0]=='V')
		orient=VERTICAL;
	else
	{
		printf("?syntax error\n\n");
		fflush(stdout);
		return;
	}	


	if(!legal_wall(b,p,orient,NO))
	{
		printf("?illegal placement\n\n");
		fflush(stdout);
		return;
	}

	if (player == WHITE)
	{
		addto_undo(BUILD,WHITE,p,adr,move_num);
		b->player1.available_walls--;
	}
	else
	{
		addto_undo(BUILD,BLACK,p,adr,move_num);
		b->player2.available_walls--;		
	}	

	printf("=\n\n");
	fflush(stdout);		
}

//token 11
int undo(listptr *adr,board *b,char *buff, int* move_num)
{
	int i,times;
	listptr tmp;
	char* token;

	token=strtok(buff,space);//COMMAND
	token=strtok(NULL,space);//TIMES

	if (token == NULL)
		times=1;
	else	
		times = atoi(token);

	for(i=0;i<times;i++)
	{
		if((*adr) == NULL)
		{
			printf("? cannot undo(max:%d times)\n\n",i);
			fflush(stdout);
			return -1;
		}

		(*move_num)--;

		if((*adr)->move == BUILD)
		{
			b->cells[(*adr)->cords.y][(*adr)->cords.x].wall = NO_WALL;

			if((*adr)->player == WHITE)
				b->player1.available_walls++;
			else if ((*adr)->player == BLACK)	
				b->player2.available_walls++;
		}

		if((*adr)->move == TRANSFER)
		{
			if((*adr)->player == WHITE)
			{
				b->cells[b->player1.position.y][b->player1.position.x].player=NO_PLAYER;
				b->player1.position.x=(*adr)->cords.x; b->player1.position.y=(*adr)->cords.y; 
				b->cells[(*adr)->cords.y][(*adr)->cords.x].player=WHITE;
			}	
			else if((*adr)->player == BLACK)
			{
				b->cells[b->player2.position.y][b->player2.position.x].player=NO_PLAYER;
				b->player2.position.x=(*adr)->cords.x; b->player2.position.y=(*adr)->cords.y; 
				b->cells[(*adr)->cords.y][(*adr)->cords.x].player=BLACK;
			}
		}

		tmp = (*adr);
		(*adr) = (*adr)->next;
		free(tmp);
	}

	printf("=\n\n");
	return 0;
}

//token 12
color winner(board b)
{
	return winner_helper(b);
} 

//token 13
void showboard(board* b)
{
	int i,j;
	int size = b->size;
	bool v_wall_edge,h_wall_edge;

	printf("=\n  ");
	for (i=0; i<size; i++) 
		printf("   %c", i+'A');//prints the col letter
	fflush(stdout);

	printf("\n   +");
	for(j=0;j<b->size;++j) printf("---+"); //prints the first line that cant have any walls

	for (i=0;i<size;i++)
	{
		printf("\n%2d |",b->size - i);  //Print the number of the line (reverse line of the array)
		for(j=0;j<size;j++)
		{
			v_wall_edge=NO;
			
			if(i>=0)
			{
				printf(" ");
				if(b->cells[i][j].player!= NO_PLAYER) //check for players in cell
				{
					if(b->cells[i][j].player == WHITE) printf("W");
					else printf("B");
				}
				else printf(" ");
				printf(" ");

				if(b->cells[i][j].wall == VERTICAL)
					v_wall_edge=YES;

				if(i>0)
					if(b->cells[i-1][j].wall == VERTICAL)
						v_wall_edge=YES;


				if(v_wall_edge) printf("H");
				else printf("|");	

				fflush(stdout);				
			}
		}
		printf("%2d",b->size - i);

		if(i==0)printf("  White walls: %d",b->player1.available_walls);
		if(i==1)printf("  Black walls: %d",b->player2.available_walls);

		printf("\n   +");  
		for(j=0;j<size;j++) //printing the horizontal lines of the board
		{
			h_wall_edge=NO;

			if(i>=0  && i<size-1)
			{
				if(b->cells[i][j].wall == HORIZONTAL) 
					h_wall_edge=YES;

				if(j>0)
					if(b->cells[i][j-1].wall == HORIZONTAL)
						h_wall_edge=YES;

				if(h_wall_edge) printf("===");
				else printf("---");
			}
			else printf("---");

			if(i>=0 && i<size)
			{
				if (b->cells[i][j].wall == HORIZONTAL) printf("=");
				else if (b->cells[i][j].wall == VERTICAL) printf("H");
				else printf("+");
			}
			else printf("+"); 

			fflush(stdout);
		}
		
		fflush(stdout);

	}

	printf("\n  ");
	for(j=0;j<b->size;++j)printf("   %c", 'A'+j);
	printf("\n");
	fflush(stdout);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void genmove(board *b, char* buff,int* move_num,listptr *adr)
{
	color player_color;
	player *agent; player* oponent;
	char* token;
	double winrate = 1;

	token=strtok(buff,space);
    token=strtok(NULL,space);

	if(token == NULL)
	{
		printf("?syntax error\n\n");
		fflush(stdout);
		return;	
	}

	if(token[0]=='w' || token[0]=='W')
		player_color=WHITE;
	else if(token[0]== 'b' || token[0]=='B')
		player_color=BLACK;
	else
	{
		printf("?syntax error\n\n");
		fflush(stdout);
		return;
	}

	if(player_color==WHITE)
	{
		agent = &(b->player1);
		oponent = &(b->player2);

	}	
	else
	{
		agent = &(b->player2);
		oponent = &(b->player1);
	}

	// heuristic: common openings
	if(*move_num <5 && oponent->position.x == 4 && oponent->position.y == 6 && ((double)rand() / RAND_MAX) < 0.5)
	{
		int BestMoves[4][3] = {
			{5,3,HORIZONTAL},
			{5,4,HORIZONTAL},
			{4,3,VERTICAL},
			{4,4,VERTICAL}
		};
		int n = rand()%4;
		point effect;

		agent->available_walls--;
		b->cells[BestMoves[n][0]][BestMoves[n][1]].wall=BestMoves[n][2];

		effect.y =BestMoves[n][0]; effect.x=BestMoves[n][1];
		addto_undo(BUILD,agent->player_color,effect,adr,move_num);	

		char vertex[2];
		cords_to_vertex(effect,vertex,b);
		printf("= %s",vertex);
		
		if(BestMoves[n][2] == HORIZONTAL)
			printf(" h\n\n");
		else
			printf(" v\n\n");		
		
		return;
	}

	if(*move_num <5 && oponent->position.x == 4 && oponent->position.y == 2 && ((double)rand() / RAND_MAX) < 0.5)
	{
		int BestMoves[4][3] = {
			{2,3,HORIZONTAL},
			{2,4,HORIZONTAL},
			{3,3,VERTICAL},
			{3,4,VERTICAL}
		};
		int n = rand()%4;
		point effect;

		agent->available_walls--;
		b->cells[BestMoves[n][0]][BestMoves[n][1]].wall=BestMoves[n][2];

		effect.y =BestMoves[n][0]; effect.x=BestMoves[n][1];
		addto_undo(BUILD,agent->player_color,effect,adr,move_num);	

		char vertex[2];
		cords_to_vertex(effect,vertex,b);
		printf("= %s",vertex);

		if(BestMoves[n][2] == HORIZONTAL)
			printf(" h\n\n");
		else
			printf(" v\n\n");

		return;
	}
	
	
	// heuristic:
	// For initial phase of a game, moves are difficult for AI , so help AI.
	// And if AI is loosing seriously, help it as well.
	// So, if it is initial phase of a game or estimated winRate is low enough,
	// help AI to find shortest path pawn move.
	if(agent->available_walls>0 && (*move_num>6 || agent->position.x != b->size/2))
	{
		winrate = MCTS(b,player_color,adr,move_num);
		printf("Thread winnrate:%f\n",winrate);
		fflush(stdout);
		if(winrate>=0.1) return; 
	}

	point current = FindShortestorLongestPath(b,player_color,SHORTEST);

	addto_undo(TRANSFER,agent->player_color,agent->position,adr,move_num);
	b->cells[agent->position.y][agent->position.x].player=NO_PLAYER;
	b->cells[current.y][current.x].player=agent->player_color;
	agent->position.x=current.x;
	agent->position.y=current.y;

	char vertex[3];
	cords_to_vertex(current,vertex,b);
	printf("= %s\n\n",vertex);

}