#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quoridor_dfs.h"


int get_command_id(char* buff);
void clean_input(char *buff);
point vertex_to_cordinates(char* vertex,int size);
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


int legal_wall(board *b,point p,orientation orient,bool no_place)
{
	bool result = NO;
	
	if(p.x>b->size-2 || p.y>b->size-2 || p.x<0 || p.y<0)
		return NO;

	if(b->cells[p.y][p.x].wall == NO_WALL)
	{
		if(orient == HORIZONTAL && (p.x+1>b->size-2 || b->cells[p.y][p.x+1].wall != HORIZONTAL) && (p.x-1<0 || b->cells[p.y][p.x-1].wall != HORIZONTAL) )
			result = YES;
		if(orient == VERTICAL && (p.y+1>b->size-2 || b->cells[p.y+1][p.x].wall != VERTICAL) && (p.y-1<0 || b->cells[p.y-1][p.x].wall != VERTICAL))
			result = YES;
	}
	if(result)
	{
		if(!touching_wall(b,p,orient))
		{
			if(!no_place)
				b->cells[p.y][p.x].wall=orient;
			return YES;		
		}

		b->cells[p.y][p.x].wall=orient;
		if(illegal_wall_check(b,WHITE))
			if(illegal_wall_check(b,BLACK))
			{
				if(no_place)
					b->cells[p.y][p.x].wall=NO_WALL;
				return YES;
			}
			else
				b->cells[p.y][p.x].wall=NO_WALL;	
		else
			b->cells[p.y][p.x].wall=NO_WALL;
	}													
	return NO;
}

int touching_wall(board* b,point p,orientation orient)
{
	if(orient == HORIZONTAL)
	{
		if(p.x == 0 || p.x == b->size - 2) return YES;

		//if(p.x == 1 || p.x == b->size - 1) return NO;

		//left
		if(p.y != 0 && b->cells[p.y-1][p.x-1].wall == VERTICAL) return YES;
		if(p.y != b->size-2 && b->cells[p.y+1][p.x-1].wall == VERTICAL) return YES;
		if(b->cells[p.y][p.x-1].wall == VERTICAL ||b->cells[p.y][p.x-2].wall == HORIZONTAL) return YES;

		//middle
		if(p.y != 0 && b->cells[p.y-1][p.x].wall == VERTICAL) return YES;
		if(p.y != b->size-2 && b->cells[p.y+1][p.x].wall == VERTICAL) return YES;

		//right
		if(p.y != 0 && b->cells[p.y-1][p.x+1].wall == VERTICAL) return YES;
		if(p.y != b->size-2 && b->cells[p.y+1][p.x+1].wall == VERTICAL) return YES;
		if(b->cells[p.y][p.x+1].wall == VERTICAL ||b->cells[p.y][p.x+2].wall == HORIZONTAL) return YES;
		
	}
	else
	{
		if(p.y == 0 || p.y == b->size - 2) return YES;

		//if(p.y == 1 || p.y == b->size - 1) return NO;	
	
		//up
		if(p.x != 0 && b->cells[p.y-1][p.x-1].wall == HORIZONTAL) return YES;
		if(p.x != b->size-2 && b->cells[p.y-1][p.x+1].wall == HORIZONTAL) return YES;
		if(b->cells[p.y-1][p.x].wall == HORIZONTAL) return YES;
		if(p.y>1 && b->cells[p.y-2][p.x].wall == VERTICAL) return YES;


		//middle
		if(p.x != 0 && b->cells[p.y][p.x-1].wall == HORIZONTAL) return YES;
		if(p.x != b->size-2 && b->cells[p.y][p.x+1].wall == HORIZONTAL) return YES;

		//down
		if(p.x != 0 && b->cells[p.y+1][p.x-1].wall == HORIZONTAL) return YES;
		if(p.y != b->size-2 && b->cells[p.y+1][p.x+1].wall == HORIZONTAL) return YES;
		if(b->cells[p.y-1][p.x].wall == HORIZONTAL) return YES;
		if(p.y < b->size-3 && b->cells[p.y+2][p.x].wall == VERTICAL) return YES;
	}

	return NO;
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
	point temp_p;

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

void print_childs(tlptr list)
{
	tree_node child;
	int n=0;

	while(list != NULL)
	{
		n++;
		child = *(list->child);


		list=list->next;
	}
}

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