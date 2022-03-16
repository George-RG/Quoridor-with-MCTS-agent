#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quoridor_structs.h"

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