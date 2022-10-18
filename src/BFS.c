#include <stdio.h>
#include <stdlib.h>

#include "BFS.h"

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