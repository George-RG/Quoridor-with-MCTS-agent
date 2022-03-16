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
#include "quoridor_helpfull_commands.h"
#include "dijkstra_s.h"


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



//BFS pathfinding
int BFS(struct Graph* graph, int startBlock,int target,board *b);
struct queue* createQueue();
int isEmpty(struct queue* q);
void enqueue(struct queue* q, int value);
int dequeue(struct queue* q);
void printQueue(struct queue* q);
int find_min_steps(board* b,color player,point start,int target);
void find_shortest_or_longest_path (board *b,color cur_agent,distance distance,point* next_moves);


//MCTS
double MCTS(board *original_board,color player,int sims,listptr* adr, int* move_num);
tree_node* findPromisingNode(tree_node *root,board* b);
tree_node* FindBestChild(tree_node node);
tree_node* FindBestChild_PASSES(tree_node node);
tree_node* GetNth(int n, tlptr list);
double GetUCT(int parent_visits,tree_node node);
void expand_node(tree_node *node , board *b);
void PerformMove(tree_node node,board *b);
void add_child(tree_node *node,color player,move move,point p,orientation orient);
int SimulateRollout(tree_node node,board board);
bool Heuristic_Move(color cur_agent,board *b,bool no_wall);
point find_probable_next_wall(board *b,color cur_agent,orientation* orient_return);
point find_all_next_wall(board *b,color cur_agent,orientation* orient_return);
void backPropagation(tree_node *node, int sim_winner,color player);
void free_childs(tree_node* root);
int evaluate(board b);

//TEST
void genmove(board *b, char* buff,int *move_num,listptr* adr);



#pragma endregion Function_declerretion

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

#pragma region Command_Functions
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
	if(b.player1.position.y==0)
	{
		return WHITE;
	}
	else if(b.player2.position.y==b.size-1)
	{
		return BLACK;
	}
	return NO_PLAYER;
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





#pragma endregion Command_Functions


#pragma region BFS

// BFS algorithm
int BFS(struct Graph* graph, int startBlock,int target,board *b) 
{
    struct queue* q1 = createQueue();
    struct queue* q2 = createQueue();
    struct queue* q_cur;
    struct queue* q_next;
    int n=1,steps=0;
    int currentBlock,adjBlock,temp_block;
    struct stacknode* temp;
	point adj_p;
	point cur_p;

    graph->visited[startBlock] = 1;
    enqueue(q1, startBlock);

    while (!isEmpty(q1) || !isEmpty(q2)) 
    {
        n=(n+1)%2;
        if (n==0) 
        {
            q_cur = q1; 
            q_next = q2;
        }    
        else 
        {
            q_cur = q2;
            q_next = q1;
        }

        while(!isEmpty(q_cur))
        {
            currentBlock = dequeue(q_cur);
            
			cur_p = ntp(b->size,currentBlock);

			if (cur_p.y==target) break;

			if(!wall_left(cur_p,b) && cur_p.x != 0)
			{
				adj_p.x=cur_p.x-1;adj_p.y=cur_p.y;
				temp_block=ptn(adj_p,b->size);
				if(!graph->visited[temp_block])
					addEdge(graph,currentBlock,temp_block);
			}

			if(!wall_right(cur_p,b) && cur_p.x != b->size-1)
			{
				adj_p.x=cur_p.x+1;adj_p.y=cur_p.y;
				temp_block=ptn(adj_p,b->size);
				if(!graph->visited[temp_block])
					addEdge(graph,currentBlock,temp_block);
			}

			if(!wall_up(cur_p,b) && cur_p.y != 0)
			{
				adj_p.x=cur_p.x;adj_p.y=cur_p.y-1;
				temp_block=ptn(adj_p,b->size);
				if(!graph->visited[temp_block])
					addEdge(graph,currentBlock,temp_block);
			}
	
			if(!wall_down(cur_p,b) && cur_p.y != b->size-1)
			{
				adj_p.x=cur_p.x;adj_p.y=cur_p.y+1;
				temp_block=ptn(adj_p,b->size);
				if(!graph->visited[temp_block])
					addEdge(graph,currentBlock,temp_block);
			}
			

            temp = graph->adjLists[currentBlock];

            while (temp) {
                adjBlock = temp->block;

                if (graph->visited[adjBlock] == 0) {
                    graph->visited[adjBlock] = 1;
                    enqueue(q_next, adjBlock);
                }
                temp = temp->next;
            }
        } 
        if (cur_p.y==target) break;  
        steps++;
    }
	free(q1);
	free(q2);
    return steps;
}

// Create a queue
struct queue* createQueue() 
{
  struct queue* q = malloc(sizeof(struct queue));
  q->front = -1;
  q->rear = -1;
  return q;
}

// Check if the queue is empty
int isEmpty(struct queue* q) 
{
  if (q->rear == -1)
    return 1;
  else
    return 0;
}

// Adding elements into queue
void enqueue(struct queue* q, int value) 
{
  if (q->rear < 40 - 1) 
  {
		if (q->front == -1)
			q->front = 0;
		q->rear++;
		q->items[q->rear] = value;
  }
}

// Removing elements from queue
int dequeue(struct queue* q) 
{
  int item;
  if (isEmpty(q)) {
    item = -1;
  } else {
    item = q->items[q->front];
    q->front++;
    if (q->front > q->rear) {
      q->front = q->rear = -1;
    }
  }
  return item;
}

// Print the queue
void printQueue(struct queue* q) 
{
  int i = q->front;

  if (isEmpty(q)) {
    printf("Queue is empty");
  } else {
    printf("\nQueue contains :");
    for (i = q->front; i < q->rear + 1; i++) {
      printf("%d ", q->items[i]);
    }
    printf("\n");
  }
}

int find_min_steps(board* b,color player,point start,int target)
{
	int result,i;
	struct Graph* graph = createGraph(b->size * b->size);

	result = BFS(graph,ptn(start,b->size),target,b);

	for(i=0;i<b->size * b->size;i++)
	{
		free_list_DFS(graph->adjLists[i]);
	}
	free(graph->adjLists);
	free(graph->visited);
	free(graph);


	return result;
}

void find_shortest_or_longest_path (board *b,color cur_agent,distance distance, point* next_moves)
{
	int i,dislocation;
	player agent,oponent;
	point start;
	int current_best[3];
	int temp_score[3];
	current_best[0]=0;current_best[1]=0;
	bool wall_between;
	int target;

	if(distance == SHORTEST)
	{	
		current_best[2]=100000;
	}
	else
	{
		current_best[2]=-1;	
	}

	//Agent going up and down
	if(cur_agent == WHITE)
	{
		agent = b->player1;
		oponent = b->player2;
		dislocation = 1;
		target =0;
	}
	else
	{
		agent = b->player2;
		oponent = b->player1;
		dislocation = -1;
		target = b->size-1;
	}

	
	//agent going up and down
	for(i=0;i<2;i++)
	{
		dislocation*=-1;
		start = agent.position;
		start.y+=dislocation;

		if(dislocation == -1)
			wall_between = wall_up(agent.position,b);
		else
			wall_between = wall_down(agent.position,b);	

		if(legal_move(b,start,&agent,&oponent))
		{
			temp_score[2]=find_min_steps(b,agent.player_color,start,target); temp_score[0]=start.y; temp_score[1]=start.x;
			compare_paths(current_best,temp_score,distance,next_moves);
		}
		else if(oponent.position.x == start.x && oponent.position.y == start.y && !wall_between)
		{
			start.y+=dislocation;
			if(legal_move(b,start,&agent,&oponent))
			{
				temp_score[2]=find_min_steps(b,agent.player_color,start,target); temp_score[0]=start.y; temp_score[1]=start.x;
				compare_paths(current_best,temp_score,distance,next_moves);
			}
			else if(start.y>0 && start.y<b->size-1)
			{
				start.y-=dislocation;
				start.x--;
				if(legal_move(b,start,&agent,&oponent))
				{
					temp_score[2]=find_min_steps(b,agent.player_color,start,target); temp_score[0]=start.y; temp_score[1]=start.x;
					compare_paths(current_best,temp_score,distance,next_moves);
				}
				start.x+=2;
				if(legal_move(b,start,&agent,&oponent))
				{
					temp_score[2]=find_min_steps(b,agent.player_color,start,target); temp_score[0]=start.y; temp_score[1]=start.x;
					compare_paths(current_best,temp_score,distance,next_moves);
				}
			}
		}
	}

	//Agent going left and right
	for(i=0;i<2;i++)
	{
		dislocation*=-1;
		start = agent.position;
		start.x+=dislocation;

		if(dislocation == -1)
			wall_between = wall_left(agent.position,b);
		else
			wall_between = wall_right(agent.position,b);	

		if(legal_move(b,start,&agent,&oponent))
		{
			temp_score[2]=find_min_steps(b,agent.player_color,start,target); temp_score[0]=start.y; temp_score[1]=start.x;
			compare_paths(current_best,temp_score,distance,next_moves);
		}
		else if(oponent.position.x == start.x && oponent.position.y == start.y && !wall_between)
		{
			start.x+=dislocation;
			if(legal_move(b,start,&agent,&oponent))
			{
				temp_score[2]=find_min_steps(b,agent.player_color,start,target); temp_score[0]=start.y; temp_score[1]=start.x;
				compare_paths(current_best,temp_score,distance,next_moves);
			}
			else if(start.x>0 && start.x<b->size-1)
			{
				start.x-=dislocation;
				start.y--;
				if(legal_move(b,start,&agent,&oponent))
				{
					temp_score[2]=find_min_steps(b,agent.player_color,start,target); temp_score[0]=start.y; temp_score[1]=start.x;
					compare_paths(current_best,temp_score,distance,next_moves);
				}
				start.y+=2;
				if(legal_move(b,start,&agent,&oponent))
				{
					temp_score[2]=find_min_steps(b,agent.player_color,start,target); temp_score[0]=start.y; temp_score[1]=start.x;
					compare_paths(current_best,temp_score,distance,next_moves);
				}
			}
		}
	}
}

#pragma endregion BFS


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double MCTS(board *original_board,color player,int sims,listptr* adr, int* move_num)
{
	board b = boardcpy(*original_board);;
	tree_node *root = malloc(sizeof(tree_node)); 
	root->wins=0; root->passes=0; root->visited=YES; root->parent=NULL; root->childs_list=NULL;

	if(player == WHITE)
		root->current_move.player=BLACK;
	else
		root->current_move.player=WHITE;
	
	int simulations=0;
	tree_node *PromisingNode;
	tree_node *nodeToExplore;
	tree_node *temp;
	color rolloutResult;

	while(simulations < sims)
	{
		simulations++;
		free_board(b);
		b = boardcpy(*original_board);

		//Phase 1 - Selection
		temp = findPromisingNode(root,&b);
		if(temp != NULL)
			PromisingNode = temp;

		//Phase 2 - Expand
		if(winner(b) == NO_PLAYER && PromisingNode->visited == YES)
		{
			expand_node(PromisingNode,&b);
		}

		//Phase 3 - Simulate
		nodeToExplore = PromisingNode;
		if (nodeToExplore->childs_list != NULL)
		{
			temp = SelectRandomChild(*PromisingNode);
			if(temp != NULL)
			{
				nodeToExplore = temp;
				PerformMove(*nodeToExplore,&b);
			}	
		}
		nodeToExplore->visited=YES;

		rolloutResult = SimulateRollout(*nodeToExplore,b);

		//Phase 4 - Update
		backPropagation(nodeToExplore,rolloutResult,player);

		// if(simulations % 1000 == 0)
		// 	printf("		Simulation:%d		",simulations);

	}
	free_board(b);
	tree_node* BestNode = FindBestChild_PASSES(*root);

	double winRate = (double)(BestNode->wins)/(double)(BestNode->passes);
	if(winRate<0.1)
		return winRate;

	if(BestNode->current_move.move == TRANSFER)
		if(player == WHITE)
			addto_undo(BestNode->current_move.move,BestNode->current_move.player,original_board->player1.position,adr,move_num);
		else
			addto_undo(BestNode->current_move.move,BestNode->current_move.player,original_board->player2.position,adr,move_num);
	else
		addto_undo(BestNode->current_move.move,BestNode->current_move.player,BestNode->current_move.point,adr,move_num);	

	PerformMove(*BestNode,original_board);

	char vertex[2];
	cords_to_vertex(BestNode->current_move.point,vertex,original_board);
	printf("= %s",vertex);

	if(BestNode->current_move.orient == HORIZONTAL)
		printf(" h");
	else if (BestNode->current_move.orient == VERTICAL)
		printf(" v");	
	
	printf("\n\n");

	free_childs(root);

	return winRate;
}

#pragma region Phase_1

tree_node* findPromisingNode(tree_node *root,board* b)
{
	tree_node *node = root;
	tlptr temp_ptr = node->childs_list;
	int n=0;
	

	while(temp_ptr != NULL)
	{
		n++;
		node = FindBestChild(*node);
		PerformMove(*node,b);
		temp_ptr = node->childs_list;
	}
	return node;
}

tree_node* FindBestChild(tree_node node)
{
	int parent_visits = node.passes;
	tlptr temp_ptr = node.childs_list;
	tree_node child;

	int n=0;
	int max_num=0;
	double maxUCT=0;
	double cur_uct;

	double test;

	while(temp_ptr != NULL)
	{
		n++;
		child = *(temp_ptr->child);

		cur_uct = GetUCT(parent_visits,child);
		if(maxUCT < cur_uct)
		{
			maxUCT = cur_uct;
			max_num = n;
		}

		temp_ptr=temp_ptr->next;
	}

	return GetNth(max_num,node.childs_list);
}

tree_node* FindBestChild_PASSES(tree_node node)
{
	tlptr temp_ptr = node.childs_list;
	tree_node child;

	int n=0;
	int max_num=0;
	double maxPasses=0;
	double cur_passes;

	double test;

	while(temp_ptr != NULL)
	{
		n++;
		child = *(temp_ptr->child);

		cur_passes = child.passes;
		if(maxPasses < cur_passes)
		{
			maxPasses = cur_passes;
			max_num = n;
		}

		temp_ptr=temp_ptr->next;
	}

	return GetNth(max_num,node.childs_list);
}

tree_node *GetNth(int n, tlptr list)
{
	int i;
	for(i=1;i<n;i++)
	{
		list = list->next;
	}
	return list->child;
}

double GetUCT(int parent_visits,tree_node node)
{
	if(node.visited == NO)
	{
		return 1e+37;
	}

	double temp = ((double)node.wins / (double)node.passes) + sqrt(0.5 * log((double)parent_visits) / (double)node.passes);
	return temp;
}

void PerformMove(tree_node node,board *b)
{
	player *p;
	if(node.current_move.player == WHITE)
		p = &b->player1;
	else
		p = &b->player2;	

	if(node.current_move.move == TRANSFER)
	{
		b->cells[p->position.y][p->position.x].player=NO_PLAYER;
		b->cells[node.current_move.point.y][node.current_move.point.x].player=p->player_color;
		p->position.x=node.current_move.point.x;
		p->position.y=node.current_move.point.y;		
	}
	else
	{
		p->available_walls--;
		b->cells[node.current_move.point.y][node.current_move.point.x].wall=node.current_move.orient;	
	}
}

#pragma endregion

#pragma region Phase_2
void expand_node(tree_node *node , board *b)
{
	player agent,oponent;
	point start;
	int dislocation; //optimise first add the move to bring the agent near the end of the game
	int i;

	if(node->current_move.player == BLACK)
	{
		agent = b->player1;
		oponent = b->player2;
		dislocation = 1;
	}
	else
	{
		agent = b->player2;
		oponent = b->player1;
		dislocation = -1;
	}
	
	if(oponent.available_walls>0)
	{	
		//possible walls
		point test_p;
		int dist = 1;
		int sign=-1;
		test_p.x=b->size/2;test_p.y=b->size/2;
		if(agent.available_walls>0)//Every posible wall placement
		{
			do
			{	
				for(i=0;i<dist;i++)
				{
					test_p.y+=sign;
					if(legal_wall(b,test_p,HORIZONTAL,YES))
						add_child(node,agent.player_color,BUILD,test_p,HORIZONTAL);
					if(legal_wall(b,test_p,VERTICAL,YES))
						add_child(node,agent.player_color,BUILD,test_p,VERTICAL);
				}
				for(i=0;i<dist;i++)
				{
					test_p.x+=sign;
					if(legal_wall(b,test_p,HORIZONTAL,YES))
						add_child(node,agent.player_color,BUILD,test_p,HORIZONTAL);
					if(legal_wall(b,test_p,VERTICAL,YES))
						add_child(node,agent.player_color,BUILD,test_p,VERTICAL);
				}
				dist++;
				sign*=-1;
			}while(dist<b->size-1);

			for(i=0;i<dist-1;i++)
			{
				test_p.y++;
				if(legal_wall(b,test_p,HORIZONTAL,YES))
					add_child(node,agent.player_color,BUILD,test_p,HORIZONTAL);
				if(legal_wall(b,test_p,VERTICAL,YES))
					add_child(node,agent.player_color,BUILD,test_p,VERTICAL);
			}
		}	

		//Agent going up and down
		for(i=0;i<2;i++)
		{
			dislocation*=-1;
			start = agent.position;
			start.y+=dislocation;
			if(legal_move(b,start,&agent,&oponent))
				add_child(node,agent.player_color,TRANSFER,start,NO_WALL);

			else if(oponent.position.x == start.x && oponent.position.y == start.y)
			{
				start.y+=dislocation;
				if(legal_move(b,start,&agent,&oponent))
					add_child(node,agent.player_color,TRANSFER,start,NO_WALL);
				else if (start.y>0 && start.y<b->size-1)
				{
					start.y-=dislocation;
					start.x--;
					if(legal_move(b,start,&agent,&oponent))
						add_child(node,agent.player_color,TRANSFER,start,NO_WALL);
					start.x+=2;
					if(legal_move(b,start,&agent,&oponent))
						add_child(node,agent.player_color,TRANSFER,start,NO_WALL);
				}
			}
		}

		//Agent going left and right
		for(i=0;i<2;i++)
		{
			dislocation*=-1;
			start = agent.position;
			start.x+=dislocation;
			if(legal_move(b,start,&agent,&oponent))
				add_child(node,agent.player_color,TRANSFER,start,NO_WALL);
			else if(oponent.position.x == start.x && oponent.position.y == start.y)
			{
				start.x+=dislocation;
				if(legal_move(b,start,&agent,&oponent))
					add_child(node,agent.player_color,TRANSFER,start,NO_WALL);
				else if (start.x>0 && start.x<b->size-1)
				{
					start.x-=dislocation;
					start.y--;
					if(legal_move(b,start,&agent,&oponent))
						add_child(node,agent.player_color,TRANSFER,start,NO_WALL);
					start.y+=2;
					if(legal_move(b,start,&agent,&oponent))
						add_child(node,agent.player_color,TRANSFER,start,NO_WALL);
				}
			}
		}
	}
	else
	{
		// heuristic:
		// If opponent has no walls left,
		// my pawn moves only to one of the shortest paths.	
		point next_moves[5];
		find_shortest_or_longest_path(b,agent.player_color,SHORTEST,next_moves);
		for(i=0;i<5;i++)
		{
			if(next_moves[i].x == -1) break;

			add_child(node,agent.player_color,TRANSFER,next_moves[i],NO_WALL);
		}

		if(agent.available_walls>0)
		{
			// heuristic:
			// if opponent has no walls left,
			// place walls only to interrupt the opponent's path,
			// not to support my pawn.	

			point next_moves[5];
			find_shortest_or_longest_path(b,oponent.player_color,SHORTEST,next_moves);
			for(i=0;i<5;i++)
			{
				if(next_moves[i].x == -1) break;

				start = next_moves[i];

				if(legal_wall(b,start,HORIZONTAL,YES))
					add_child(node,agent.player_color,BUILD,start,HORIZONTAL);
				if(legal_wall(b,start,VERTICAL,YES))
					add_child(node,agent.player_color,BUILD,start,VERTICAL);

				start.y--;

				if(legal_wall(b,start,HORIZONTAL,YES))
					add_child(node,agent.player_color,BUILD,start,HORIZONTAL);
				if(legal_wall(b,start,VERTICAL,YES))
					add_child(node,agent.player_color,BUILD,start,VERTICAL);	

				start.x--;
				start.y++;	

				if(legal_wall(b,start,HORIZONTAL,YES))
					add_child(node,agent.player_color,BUILD,start,HORIZONTAL);
				if(legal_wall(b,start,VERTICAL,YES))
					add_child(node,agent.player_color,BUILD,start,VERTICAL);
			}
		}
	}
}


void add_child(tree_node *node,color player,move move,point p,orientation orient)
{
	tlptr temp_ptr = node->childs_list;
	node->childs_list=malloc(sizeof(tl_node));
	node->childs_list->next=temp_ptr;

	node->childs_list->child=malloc(sizeof(tree_node));
	
	tree_node *child=node->childs_list->child;

	child->wins = 0; child->passes = 0; child->visited = NO; 
	child->parent = node;
	child->childs_list = NULL;

	child->current_move.player = player;
	child->current_move.move = move;
	child->current_move.point = p;
	child->current_move.orient = orient;
}

void free_childs(tree_node* root)
{	
	tlptr list = root->childs_list;
	tlptr temp;
	while(list != NULL)
	{
		free_childs(list->child);
		temp = list;
		list = list->next;
		free(temp);
	}
	free(root);
}

#pragma endregion Phase_2

#pragma region Phase_3
int SimulateRollout(tree_node node,board board)
{
	int board_winner = winner(board);
	color current_player = node.current_move.player;
	int heuristic_moves=0;
	bool no_wall_bool = NO;

	while (board_winner == NO_PLAYER && heuristic_moves < 40)
	{
		heuristic_moves++;
		current_player = ((current_player)%2)+1; //toggle player

		no_wall_bool = Heuristic_Move(current_player,&board,no_wall_bool);
		//showboard(&board);
		board_winner = winner(board);
	}

	if(board_winner == NO_PLAYER)
	{	
		int eval = evaluate(board);
		if(eval>0)
			board_winner=WHITE;
		else
			board_winner=BLACK;	
	}

	return board_winner;
}


int evaluate(board b)
{
	int white_walls = b.player1.available_walls;
	int black_walls = b.player2.available_walls;

	int white_next_line = find_min_steps(&b,WHITE,b.player1.position,b.player1.position.y-1);
	int black_next_line = find_min_steps(&b,BLACK,b.player2.position,b.player2.position.y+1);

	int white_finish = find_min_steps(&b,WHITE,b.player1.position,0);
	int black_finish = find_min_steps(&b,BLACK,b.player2.position,b.size-1);

	return (10*(white_finish-black_finish) + 6*(white_next_line-black_next_line) + 8*(white_walls-black_walls));
}

bool Heuristic_Move(color cur_agent,board *b,bool no_wall)
{
	player agent,oponent;
	player *p;
	point current;
	current.x=-1;current.y=-1;
	point next_moves[5]={current,current,current,current,current};
	move move_type;
	orientation orient;

	if(cur_agent == WHITE)
	{
		agent = b->player1;
		oponent = b->player2;
		p = &b->player1;
	}
	else
	{
		agent = b->player2;
		oponent = b->player1;
		p = &b->player2;
	}

	if( ((double)rand() / RAND_MAX) < 0.7 )
	{
		//find_shortest_or_longest_path(b,cur_agent,SHORTEST,next_moves);
		
		//current = next_moves[0];
		current = FindShortestorLongestPath(b,cur_agent,SHORTEST);
		move_type=TRANSFER;
	}
	else if(agent.available_walls>0 && /*(((double)rand() / RAND_MAX) < 0.7)*/ !no_wall)
	{
		double chance = ((double)rand() / RAND_MAX);
		bool no_good=NO;
		if( chance < 0.8 )
		{
			current = find_probable_next_wall(b,cur_agent,&orient);
			move_type=BUILD;
			if(current.x == -1 && current.y==-1)
				no_good = YES;
		}
		
		if(chance >= 0.8 || no_good)
		{
			current = find_all_next_wall(b,cur_agent,&orient);
			move_type=BUILD;
			if(current.x = -1)
			{
				return YES;
			}
		}
	}
	else
	{
		// find_shortest_or_longest_path(b,cur_agent,LONGEST,next_moves);
		// current = next_moves[0];

		current = FindShortestorLongestPath(b,cur_agent,LONGEST);

		move_type=TRANSFER;
	}

	if(current.x != -1 && current.y!=-1)
	{
		if(move_type == TRANSFER)
		{
			b->cells[p->position.y][p->position.x].player=NO_PLAYER;
			b->cells[current.y][current.x].player=p->player_color;
			p->position.x=current.x;
			p->position.y=current.y;	
			return NO;	
		}
		else
		{
			p->available_walls--;
			b->cells[current.y][current.x].wall=orient;	
			return NO;
		}
	}	
}

point find_probable_next_wall(board *b,color cur_agent,orientation* orient_return) 
{
	int index;
	pointptr H=NULL;
	pointptr V=NULL;
	int Hlen,Vlen;
	player agent,oponent;
	bool found=NO;
	point test_p;
	orientation orient;

	if(cur_agent == WHITE)
	{
		agent = b->player1;
		oponent = b->player2;
	}
	else
	{
		agent = b->player2;
		oponent = b->player1;
	}

	point start = oponent.position;

	Hlen=add_pointlist(start,&H);Vlen=add_pointlist(start,&V);
	start.x++;
	Hlen=add_pointlist(start,&H);Vlen=add_pointlist(start,&V);
	start.x-=2;
	Hlen=add_pointlist(start,&H);Vlen=add_pointlist(start,&V);
	if(cur_agent == BLACK)
	{
		start = agent.position;
		start.y++;
		Hlen=add_pointlist(start,&H);Vlen=add_pointlist(start,&V);
		start.y-=2;
		Vlen=add_pointlist(start,&V);
	}
	else
	{
		start = agent.position;
		start.y--;
		Hlen=add_pointlist(start,&H);Vlen=add_pointlist(start,&V);
		start.y+=2;
		Vlen=add_pointlist(start,&V);
	}

	while(!found)
	{
		if( ((double)rand() / RAND_MAX) < 0.5  && V!=NULL)
		{
			index = (rand()%Vlen)+1;
			test_p = remove_nth_pointlist(index,&V);
			orient = VERTICAL;
			Vlen--;
		} 
		else if(H!=NULL)
		{
			index = (rand()%Hlen)+1;
			test_p = remove_nth_pointlist(index,&H);
			orient = HORIZONTAL;
			Hlen--;
		}

		if(legal_wall(b,test_p,orient,YES))
			found=YES;

		if(V==NULL && H==NULL)
		{
			point error;error.x=-1;error.y=-1;
			*orient_return = 0;
			return	error;
		}	
	}

	free_pointlist(H);free_pointlist(V);
	*orient_return = orient;
	return test_p;
	
}

point find_all_next_wall(board *b,color cur_agent,orientation* orient_return)
{
	int i;
	player agent,oponent;
	pointptr H=NULL;
	pointptr V=NULL;
	int Hlen,Vlen;

	if(cur_agent == WHITE)
	{
		agent = b->player1;
		oponent = b->player2;
	}
	else
	{
		agent = b->player2;
		oponent = b->player1;
	}


	point test_p;
	int dist = 1;
	int sign=-1;
	test_p.x=b->size/2;test_p.y=b->size/2;
	do
	{	
		for(i=0;i<dist;i++)
		{
			test_p.y+=sign;
			Hlen=add_pointlist(test_p,&H);Vlen=add_pointlist(test_p,&V);
		}
		for(i=0;i<dist;i++)
		{
			test_p.x+=sign;
			Hlen=add_pointlist(test_p,&H);Vlen=add_pointlist(test_p,&V);
		}
		dist++;
		sign*=-1;
	}while(dist<b->size-1);

	for(i=0;i<dist-1;i++)
	{
		test_p.y++;
		Hlen=add_pointlist(test_p,&H);Vlen=add_pointlist(test_p,&V);
	}	

	int index;
	orientation orient;
	bool found = NO;
	while(!found)
	{
		if( ((double)rand() / RAND_MAX) < 0.5  && V!=NULL)
		{
			index = (rand()%Vlen)+1;
			test_p = remove_nth_pointlist(index,&V);
			orient = VERTICAL;
			Vlen--;
		} 
		else if(H!=NULL)
		{
			index = (rand()%Hlen)+1;
			test_p = remove_nth_pointlist(index,&H);
			orient = HORIZONTAL;
			Hlen--;
		}

		if(legal_wall(b,test_p,orient,YES)) found=YES;

		if(V==NULL && H==NULL)
		{
			point error;error.x=-1;error.y=-1;
			*orient_return = 0;
			return	error;
		}	
	}

	free_pointlist(H);free_pointlist(V);
	*orient_return = orient;
	return test_p;
}

#pragma endregion Phase_3


void backPropagation(tree_node *node, int sim_winner,color player)
{
	color current_player = node->current_move.player;
	while(node != NULL)
	{
		node->passes++;
		current_player = ((current_player)%2)+1;
		if(player == sim_winner)
			node->wins+=1;
		node=node->parent;
	}
}


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
	if(agent->available_walls>0 && (*move_num>6 || agent->position.x != b->size/2) )
	{
		winrate = MCTS(b,player_color,30000,adr,move_num);
		printf("winnrate:%f\n",winrate);
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