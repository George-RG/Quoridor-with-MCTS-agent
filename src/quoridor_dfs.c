#include <stdio.h>
#include <stdlib.h>
#include "quoridor_wallcheck.h"
//#include "quoridor_structs.h"

//DFS pathfinding
void addEdge(struct Graph* graph, int src, int dest);
int DFS(struct Graph* graph, int block,board *b,int target);
struct stacknode* createstacknode(int v);
struct Graph* createGraph(int blocks);
int illegal_wall_check(board* b,color player);
void free_list_DFS(struct stacknode* head);
void printList(struct stacknode* temp,int* visited);
point ntp(int size,int k);
int ptn(point p,int size );

// DFS algo
int DFS(struct Graph* graph, int block,board *b,int target) 
{
	struct stacknode* adjList;
	struct stacknode* temp;
	int temp_block;
	int steps=0;

	graph->visited[block] = 1;

	point cur_p = ntp(b->size,block);
	point adj_p;

	if (cur_p.y==target)
		return 1;    

	if(!wall_left(cur_p,b) && cur_p.x != 0)
	{
		adj_p.x=cur_p.x-1;adj_p.y=cur_p.y;
		temp_block=ptn(adj_p,b->size);
		if(!graph->visited[temp_block])
			addEdge(graph,block,temp_block);
	}

	if(!wall_right(cur_p,b) && cur_p.x != b->size-1)
	{
		adj_p.x=cur_p.x+1;adj_p.y=cur_p.y;
		temp_block=ptn(adj_p,b->size);
		if(!graph->visited[temp_block])
			addEdge(graph,block,temp_block);
	}

	if(!wall_up(cur_p,b) && cur_p.y != 0)
	{
		adj_p.x=cur_p.x;adj_p.y=cur_p.y-1;
		temp_block=ptn(adj_p,b->size);
		if(!graph->visited[temp_block])
			addEdge(graph,block,temp_block);
	}
	
	if(!wall_down(cur_p,b) && cur_p.y != b->size-1)
	{
		adj_p.x=cur_p.x;adj_p.y=cur_p.y+1;
		temp_block=ptn(adj_p,b->size);
		if(!graph->visited[temp_block])
			addEdge(graph,block,temp_block);
	}

	adjList = graph->adjLists[block];
	temp = adjList;

	int connectedBlock;	

	while (temp != NULL) 
	{
		connectedBlock = temp->block;

		if (graph->visited[connectedBlock] == 0) 
			steps=DFS(graph, connectedBlock,b,target);
		if (steps>0)
		{
			//printf("Step found(%d)\n",steps);
			steps++;
			break;
		}		
		temp = temp->next;
	}
	//printf("DONE with:%d\n",block);
	return steps;
}

// Create a stacknode
struct stacknode* createstacknode(int v)
{
	struct stacknode* newstacknode = malloc(sizeof(struct stacknode));
	newstacknode->block = v;
	newstacknode->next = NULL;
	return newstacknode;
}

// Add edge
void addEdge(struct Graph* graph, int src, int dest) 
{
	// Add edge from src to dest
	struct stacknode* newstacknode = createstacknode(dest);
	newstacknode->next = graph->adjLists[src];
	graph->adjLists[src] = newstacknode;

	// Add edge from dest to src
	newstacknode = createstacknode(src);
	newstacknode->next = graph->adjLists[dest];
	graph->adjLists[dest] = newstacknode;
}

// Print the graph
void printGraph(struct Graph* graph) 
{
	int v;
	for (v = 0; v < graph->numBlocks; v++) 
	{
		struct stacknode* temp = graph->adjLists[v];
		printf("\n Adjacency list of vertex %d\n ", v);
		while (temp) {
		printf("%d -> ", temp->block);
		temp = temp->next;
		}
		printf("\n");
	}
}

// Print the list
void printList(struct stacknode* temp,int* visited)
{
	while (temp) {
		printf("%d[%d]-> ", temp->block,visited[temp->block]);
		temp = temp->next;
	}
	printf("\n");
}

void free_list_DFS(struct stacknode* head)
{
   struct stacknode* tmp;

   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }
}