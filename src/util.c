#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

const char white_space[] = " \t\n"; //where to split each command's words (tokens)

char* new_protocol_commands[] = {
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

void clean_input(char *buff)
{
	int i=0,j;

	while(buff[i] != '\n')
	{
		if (buff[i] == '\t') buff[i]=' ';
		if (buff[i]<=31 || buff[i]==127)
		{
			j=i+1;
			do
			{
				buff[j-1] = buff[j];
				j++;
			}while(buff[j-1] != '\n');
		}
		i++;
	}
}

int get_command_id(char *buff) 
{    
	char temp[101];
	strcpy(temp,buff);
	char* buff_copy = temp;
	
	int i;

	while (*buff_copy != '\n' )
	{
		if(*buff_copy == ' ' || *buff_copy == '\t') // white_spaces found, ignore
			buff_copy++;
		else break;
	}

	char *token;
	token = strtok(buff_copy, white_space);   
	
	if(token != NULL)
	{
		for (i=1; i< ( sizeof(new_protocol_commands) / sizeof(char *) );  i++) //find the command via strcmp
		if (! strcmp(new_protocol_commands[i], token))		//return its id
			return i;					//(success)
	}
	return 0;							//no command found (failure)
}

point vertex_to_cordinates(char* vertex, int size)
{
    char *p;
	point result;
    p=vertex;
	
	if(strlen(p) == 2)
	{
		result.x=(int)(p[0]-'A');
		result.y=(int)(p[1] - '1');
		result.y = size-result.y;
	}
	else
	{
		result.x=(int)(p[0]-'A');
		result.y=((int)(p[1] - '1')) * 10;
		result.y+=(int)(p[2] - '1');
		result.y = size-result.y;
	}
	
	if (result.x<0 || result.x>25) //Syntax Error Detection;
		result.x = -1;
	if (result.y<0 || result.y>25)
		result.y = -1;	

	return result;
}

int legal_move(board *b,point p,player *pl,player *pl2)
{
	if(p.x>=0 && p.x<=(b->size)-1 && p.y>=0 && p.y<=(b->size)-1)
	{
		if(p.y==pl->position.y+1 && p.x==pl->position.x && !wall_down(pl->position,b) && (pl2->position.x!=p.x || pl2->position.y!=p.y))
			return YES;
		else if(p.y==pl->position.y-1 && p.x==pl->position.x && !wall_up(pl->position,b)&& (pl2->position.x!=p.x || pl2->position.y!=p.y))
			return YES;
		else if(p.y==pl->position.y && p.x==pl->position.x+1 && !wall_right(pl->position,b)&& (pl2->position.x!=p.x || pl2->position.y!=p.y))
			return YES;
		else if(p.y==pl->position.y && p.x==pl->position.x-1 && !wall_left(pl->position,b)&& (pl2->position.x!=p.x || pl2->position.y!=p.y))
			return YES;
		else if(pl2->position.x==pl->position.x && pl2->position.y+1==pl->position.y && !wall_up(pl->position,b))
		{
			if(p.y==pl->position.y-2 && p.x==pl->position.x && !wall_up(pl2->position,b))
				return YES;
			else if(p.y==pl->position.y-1 && (p.x==pl->position.x-1 || p.x==pl->position.x+1) && (pl2->position.y-1 <0 ||wall_up(pl2->position,b)))	
			{
				if(p.x==pl->position.x+1 && !wall_right(pl2->position,b))
					return YES;
				else if(p.x==pl->position.x-1 && !wall_left(pl2->position,b))	
					return YES;
			}
		}
		else if(pl2->position.x==pl->position.x && pl2->position.y-1==pl->position.y && !wall_down(pl->position,b))
		{
			if(p.y==pl->position.y+2 && p.x==pl->position.x && !wall_down(pl2->position,b))
				return YES;
			else if((pl2->position.y+1 > b->size-1 ||wall_down(pl2->position,b)) && p.y==pl->position.y+1 && (p.x==pl->position.x+1 ||p.x==pl->position.x-1))
			{
				if(p.x==pl->position.x+1 && !wall_right(pl2->position,b))
					return YES;
				else if(p.x==pl->position.x-1 && !wall_left(pl2->position,b))	
					return YES;
			}
		}
		else if(pl2->position.x+1==pl->position.x && pl2->position.y==pl->position.y && !wall_left(pl->position,b))
		{
			if(p.y==pl->position.y && p.x==pl->position.x-2 && !wall_left(pl2->position,b))
				return YES;
			else if((p.y==pl->position.y-1 || p.y==pl->position.y+1) && p.x==pl->position.x-1 && (pl2->position.x-1 <0 || wall_left(pl2->position,b)))
			{
				if(p.y==pl->position.y+1 && !wall_down(pl2->position,b))
					return YES;
				else if(p.y==pl->position.y-1 && !wall_up(pl2->position,b))	
					return YES;
			}
		}		
		else if(pl2->position.x-1==pl->position.x && pl2->position.y==pl->position.y && !wall_right(pl->position,b))
		{
			if(p.y==pl->position.y && p.x==pl->position.x+2 && !wall_right(pl2->position,b))
				return YES;
			else if((p.y==pl->position.y-1 ||p.y==pl->position.y+1) && p.x==pl->position.x+1 && (pl2->position.x+1 < b->size-1 ||wall_right(pl2->position,b)))
			{
				if(p.y==pl->position.y+1 && !wall_down(pl2->position,b))
					return YES;
				else if(p.y==pl->position.y-1 && !wall_up(pl2->position,b))	
					return YES;
			}
		}
	}
	return NO;
}

void addto_undo(move move,color player,point p,listptr* adr,int *move_num)
{
	(*move_num)++;
	listptr templist;
	templist=*adr;
	*adr=malloc(sizeof(list_node));
	(*adr)->cords.x=p.x;
	(*adr)->cords.y=p.y;
	(*adr)->player=player;
	(*adr)->move=move;
	(*adr)->next=templist;
}

void compare_paths(int *a,int *b,distance distance, point* next_moves)
{
	int i;

	if(distance == SHORTEST)
	{
		if(a[2] > b[2])
		{
			a[0] = b[0];
			a[1] = b[1];
			a[2] = b[2];
			next_moves[0].y = a[0]; next_moves[0].x = a[1];
			for(i=0;i<4;i++)
			{
				next_moves[i].y=-1;next_moves[i].x=-1;
			} 
		}
	}
	else if(distance == LONGEST)
	{
		if(a[2] < b[2])
		{
			a[0] = b[0];
			a[1] = b[1];
			a[2] = b[2];
			next_moves[0].y = a[0]; next_moves[0].x = a[1];
			for(i=0;i<4;i++)
			{
				next_moves[i].y=-1;next_moves[i].x=-1;
			} 
		}
	}	

	if(a[2] == b[2])
	{
		for (i=0;i<5;i++)
		{
			if(next_moves[i].x == -1)
			{
				next_moves[i].y = b[0];
				next_moves[i].x = b[1];
				break;
			}
		}
	}
}

int add_pointlist (point p,pointptr *list)
{
	int l=0;
	while(*list!=NULL)
	{	
		list=&((*list)->next);
		l++;
	}
	*list = malloc(sizeof(point_node));
	(*list)->next=NULL;
	(*list)->point=p;
	return l+1;
}

point remove_nth_pointlist (int n,pointptr *list)
{
	pointptr templist;
	point temp_p = {-1,-1};

	while (*list != NULL)
	{
		if (n-- == 1) 
		{ 
			temp_p = (*list)->point; 
			templist = (*list)->next; 
			free(*list);
			*list = templist;
			return temp_p;
		}
		else
			list=&((*list)->next);
	}		
	return temp_p;
}

void free_pointlist(pointptr list)
{
	pointptr temp;
	while(list!=NULL)
	{	
		temp = list;
		list=list->next;
		free(temp);
	}
}

tree_node* SelectRandomChild(tree_node node)
{
	int i = count_childs(node);
	int n = (rand()%i)+1;

	while(node.childs_list != NULL)
	{
		if(n-- == 1)
		{
			return node.childs_list->child;
		}
		else
			node.childs_list = node.childs_list->next;	
	}
	return NULL;
}

int count_childs(tree_node node)
{
	int n=0;
	while(node.childs_list != NULL)
	{
		node.childs_list = node.childs_list->next;
		n++;
	}
	return n;
}

board boardcpy(board original_board)
{
	int i,j;

	board b;
	b.size = original_board.size;
	
	b.cells = malloc(b.size * sizeof(cell*));
	for(i=0;i<b.size;i++)
	{
		b.cells[i]=malloc(b.size * sizeof(cell));
	}

	for(i=0;i<b.size;i++)
	{
		for(j=0;j<b.size;j++)
		{
			b.cells[i][j] = original_board.cells[i][j];
		}
	}

	b.player1=original_board.player1;
	b.player2=original_board.player2;
	return b;
}

void free_board(board b)
{
	int i;
	for(i=0;i<b.size;i++)
	{
		free(b.cells[i]);
	}
	free(b.cells);
}

// void print_childs(tlptr list)
// {
// 	tree_node child;
// 	int n=0;

// 	while(list != NULL)
// 	{
// 		n++;
// 		child = *(list->child);

// 		list=list->next;
// 	}
// }

void cords_to_vertex(point p, char* vertex,board *b)
{
	int y = (b->size)-(p.y);
	if(y<10)
	{
		vertex[1]=y+48;
		vertex[2]=0;
	}
	else
	{
		vertex[2]=(y%10)+48;
		vertex[1]=(y/10)+48;
	}
	vertex[0]=(p.x)+65;
}

point ntp(int size,int k)     //number to point
{
	point p;
	p.x=k % size;
	p.y=k / size; 
	return p;
}

int ptn(point p,int size )  //point to number
{
	return p.y*size+p.x;
}

// Create graph
struct Graph* createGraph(int blocks)
{
	struct Graph* graph = malloc(sizeof(struct Graph));
	graph->numBlocks = blocks;

	graph->adjLists = malloc(blocks * sizeof(struct stacknode*));

	graph->visited = malloc(blocks * sizeof(int));

	int i;
	for (i = 0; i < blocks; i++) 
	{
		graph->adjLists[i] = NULL;
		graph->visited[i] = 0;
	}
	return graph;
}

int wall_up(point p,board *b)
{
	if (p.y>0)
		if(b->cells[p.y-1][p.x].wall==HORIZONTAL) 
			return 1;
	if(p.x>0 && p.y>0)
		if(b->cells[p.y-1][p.x-1].wall==HORIZONTAL) 
			return 1;

	return 0;		
}

int wall_down(point p,board *b)
{
	if (p.y<b->size -1)
		if(b->cells[p.y][p.x].wall==HORIZONTAL) 
			return 1;
	if(p.y<b->size -1 && p.x>0)
		if(b->cells[p.y][p.x-1].wall==HORIZONTAL) 
			return 1;

	return 0;
}

int wall_left(point p,board *b)
{
	if (p.x>0)
		if(b->cells[p.y][p.x-1].wall==VERTICAL) 
			return 1;
	if(p.x>0 && p.y>0)
		if(b->cells[p.y-1][p.x-1].wall==VERTICAL) 
			return 1;

	return 0;		
}

int wall_right(point p,board *b)
{
	if (p.x<b->size -1)
		if(b->cells[p.y][p.x].wall==VERTICAL) 
			return 1;
	if(p.x<b->size -1 && p.y>0)
		if(b->cells[p.y-1][p.x].wall==VERTICAL) 
			return 1;

	return 0;		
}

int winner_helper(board b)
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

int comparePoints(void* a, void* b)
{
	point* p1 = (point*) a;
	point* p2 = (point*) b;

	if(p1->x == p2->x && p1->y == p2->y)
		return 1;

	return 0;
}

ListPtr GetAllPosibleMoves(board* b, color pl_color)
{
	ListPtr list = CreateList(comparePoints,free);
	player* agent; player* opponent;

	if(pl_color == WHITE)
	{
		agent = &b->player1;
		opponent = &b->player2;
	}
	else
	{
		agent = &b->player2;
		opponent = &b->player1;
	}

	point pos = agent->position;

	point up = {pos.x, pos.y-1}, up_up = {pos.x, pos.y-2}, up_right = {pos.x+1, pos.y-1}, up_left = {pos.x-1, pos.y-1};
	if(legal_move(b,up,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = up;
		ListInsert(result,list);
	}
	else if(legal_move(b,up_up,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = up_up;
		ListInsert(result,list);
	}
	else if(legal_move(b,up_right,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = up_right;
		ListInsert(result,list);
	}
	else if(legal_move(b,up_left,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = up_left;
		ListInsert(result,list);
	}

	point down = {pos.x, pos.y+1}, down_down = {pos.x, pos.y+2}, down_right = {pos.x+1, pos.y+1}, down_left = {pos.x-1, pos.y+1};
	if(legal_move(b,down,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = down;
		ListInsert(result,list);
	}
	else if(legal_move(b,down_down,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = down_down;
		ListInsert(result,list);
	}
	else if(legal_move(b,down_right,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = down_right;
		ListInsert(result,list);
	}
	else if(legal_move(b,down_left,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = down_left;
		ListInsert(result,list);
	}

	point right = {pos.x+1, pos.y}, right_right = {pos.x+2, pos.y}, right_up = {pos.x+1, pos.y-1}, right_down = {pos.x+1, pos.y+1};
	if(legal_move(b,right,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = right;
		ListInsert(result,list);
	}
	else if(legal_move(b,right_right,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = right_right;
		ListInsert(result,list);
	}
	else if(legal_move(b,right_up,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = right_up;
		ListInsert(result,list);
	}
	else if(legal_move(b,right_down,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = right_down;
		ListInsert(result,list);
	}

	point left = {pos.x-1, pos.y}, left_left = {pos.x-2, pos.y}, left_up = {pos.x-1, pos.y-1}, left_down = {pos.x-1, pos.y+1};
	if(legal_move(b,left,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = left;
		ListInsert(result,list);
	}
	else if(legal_move(b,left_left,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = left_left;
		ListInsert(result,list);
	}
	else if(legal_move(b,left_up,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = left_up;
		ListInsert(result,list);
	}
	else if(legal_move(b,left_down,agent,opponent))
	{
		point* result = malloc(sizeof(point));
		*result = left_down;
		ListInsert(result,list);
	}

	return list;
}

double MAX(double a,double b)
{
	if(a>b)
		return a;
	else
		return b;
}

int list_length(listptr* adr)
{
	list_node* list = *adr;
	int n=0;
	while(list != NULL)
	{
		n++;
		list=list->next;
	}
	return n;
}

void shuffle(Pointer* array, size_t size)
{
	if (size > 1) 
	{
		size_t i;
		for (i = 0; i < size - 1; i++) 
		{
			size_t j = i + rand() / (RAND_MAX / (size - i) + 1);
			Pointer t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}