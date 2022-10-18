#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quoridor_wallcheck.h"

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

int illegal_wall_check(board* b,color player)
{
	int result,target,i;
	point start;
	struct Graph* graph = createGraph(b->size * b->size);

	if(player==WHITE)
	{
		start = b->player1.position;
		target = 0;
	}	
	else
	{
		start = b->player2.position;
		target = b->size-1;	
	}

	result = DFS(graph,ptn(start,b->size),b,target);

	for(i=0;i<b->size * b->size;i++)
	{
		free_list_DFS(graph->adjLists[i]);
	}
	free(graph->adjLists);
	free(graph->visited);
	free(graph);

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