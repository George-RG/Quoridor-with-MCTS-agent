#include<stdio.h>
#include<stdlib.h>
#include "quoridor_wallcheck.h"

int** create2DIntArray(int rows ,int cols ,int value);
void free2DArray(int** A ,int rows);
point FindShortestorLongestPath(board *b, color agent,distance dist);


point FindPathToGoal(board b,color agent, int*** return_dist, point*** return_prev)
{
    int i,queue_last=1,queue_front=0,target;
    player p;
    point position;

    if(agent == WHITE)
    {    
        p = b.player1;
        target = 0;
    }
    else
    {    
        p = b.player2;
        target = b.size-1;
    }    

    int **visited = create2DIntArray(b.size,b.size,NO);
    int **dist = create2DIntArray(b.size,b.size,9999);

    point **prev = malloc(sizeof(point*) * b.size);
    for(i=0; i<b.size; i++)
        prev[i] = malloc(sizeof(point) * b.size);

    point queue[b.size * b.size];

    point error={-1,-1};

    visited[p.position.y][p.position.x] = YES;
    dist[p.position.y][p.position.x] = 0;
    prev[p.position.y][p.position.x] = error;
    queue[queue_front] = p.position;

    point NextPosition;
    int NextDist;

    while(queue_front != queue_last)
    {
        position = queue[queue_front++];
        NextDist = dist[position.y][position.x]+1;
        
        if(position.y == target)
        {
            *return_dist = dist;
            *return_prev = prev; 
            free2DArray(visited,b.size);
            return position;
        }

        if(!wall_up(position,&b) && position.y != 0)
        {
            NextPosition = position; NextPosition.y--;
            if(!visited[NextPosition.y][NextPosition.x])
            {
                dist[NextPosition.y][NextPosition.x]=NextDist;
                prev[NextPosition.y][NextPosition.x]=position;
                visited[NextPosition.y][NextPosition.x]=YES;
                queue[queue_last++]=NextPosition;
            }
        }

        if(!wall_down(position,&b) && position.y != b.size-1)
        {
            NextPosition = position; NextPosition.y++;
            if(!visited[NextPosition.y][NextPosition.x])
            {
                dist[NextPosition.y][NextPosition.x]=NextDist;
                prev[NextPosition.y][NextPosition.x]=position;
                visited[NextPosition.y][NextPosition.x]=YES;
                queue[queue_last++]=NextPosition;
            }
        }

        if(!wall_left(position,&b) && position.x!=0)
        {
            NextPosition = position; NextPosition.x--;
            if(!visited[NextPosition.y][NextPosition.x])
            {
                dist[NextPosition.y][NextPosition.x]=NextDist;
                prev[NextPosition.y][NextPosition.x]=position;
                visited[NextPosition.y][NextPosition.x]=YES;
                queue[queue_last++]=NextPosition;
            }
        }

        if(!wall_right(position,&b) && position.x!=b.size-1)
        {
            NextPosition = position; NextPosition.x++;
            if(!visited[NextPosition.y][NextPosition.x])
            {
                dist[NextPosition.y][NextPosition.x]=NextDist;
                prev[NextPosition.y][NextPosition.x]=position;
                visited[NextPosition.y][NextPosition.x]=YES;
                queue[queue_last++]=NextPosition;
            }
        }
    }

    *return_dist = dist;
    *return_prev = prev; 
    free2DArray(visited,b.size);
    return error;
}

int** create2DIntArray(int rows,int cols,int value)
{
    int i,j;
    int** A = malloc(sizeof(int *) * rows);

    for(i=0;i<rows;i++)
    {
        A[i]=malloc(sizeof(int) * cols);
        for(j=0;j<cols;j++)
            A[i][j]=value;
    }
    return A;
}

void free2DArray(int** A ,int rows)
{
    int i;
    for(i=0; i<rows; i++)
        free(A[i]);
    free(A);
}

point FindShortestorLongestPath(board *b, color agent,distance dist)
{
    int i;
    int** dist_array;
    point** path_array;
    point goal;
    player p;

    if(agent == WHITE)
        p = b->player1;
    else
        p = b->player2;

    if(dist == SHORTEST)
    {
        goal = FindPathToGoal(*b,agent,&dist_array,&path_array);
        point previous = goal;
        //printf("new goal:%d,%d , color:%d\n",p.position.y,p.position.x,p.player_color);
        while(previous.x != p.position.x || previous.y != p.position.y)
        {
            //printf("goal:%d,%d prev:%d,%d\n",goal.y,goal.x,previous.y,previous.x);
            goal = previous;
            previous = path_array[previous.y][previous.x];
        }
    }
    else
    {
        int i,j;
        goal = FindPathToGoal(*b,agent,&dist_array,&path_array);
        int max_dist=0;

        for(i=0;i<b->size;i++)
        {
            for(j=0;j<b->size;j++)
            {
                if(dist_array[i][j] != 9999 && dist_array[i][j] >= max_dist)
                {
                    max_dist = dist_array[i][j];
                    goal.y=i;goal.x=j;
                }
            }
        }

        point previous = goal;
        while(previous.x != p.position.x || previous.y != p.position.y)
        {
            goal = previous;
            previous = path_array[goal.y][goal.x];
        }
    }

    free2DArray(dist_array,b->size);

    for(i=0; i<b->size; i++)
        free(path_array[i]);
    free(path_array);

    return goal;
}