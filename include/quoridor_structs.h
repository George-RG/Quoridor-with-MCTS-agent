#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
	int x;
	int y;
} point;

typedef enum {
	NO_PLAYER, WHITE, BLACK
} color;

typedef enum {
	NO_WALL, HORIZONTAL, VERTICAL
} orientation;

typedef enum{
	NO,YES
} bool;

typedef enum{
	TRANSFER,BUILD
} move;

typedef enum{
	SHORTEST,LONGEST
} distance;

typedef struct {
	point position;
	color player_color;
	int available_walls;
} player ;

typedef struct {
	orientation wall;
	color player;
} cell;

typedef struct {
	player player1;
	player player2;

	int size;
	cell** cells;

} board;

typedef struct point_node{
	point point;
	struct point_node* next;
} point_node;

typedef point_node* pointptr;

//DFS && BFS
struct stacknode {
	int block;
	struct stacknode* next;
};

struct Graph {
	int numBlocks;
	int* visited;

	// We need int** to store a two dimensional array.
	// Similary, we need struct stacknode** to store an array of Linked lists
	struct stacknode** adjLists;
};

typedef struct list_node{
	point cords;
	color player;
	move move;

	struct list_node* next;
}list_node;

typedef list_node* listptr;

struct queue {
  int items[100];
  int front;
  int rear;
};

//MCTS tree strructure
typedef struct mcts_move{
	color player;
	move move;
	point point;
	orientation orient;
}mcts_move;

typedef struct tl_node{ //tree list
	struct tree_node* child;
	struct tl_node* next;
}tl_node;

typedef tl_node* tlptr; //tree list ptr

typedef struct tree_node{
	double wins;
	int passes;
	//double utc;
	bool visited;
	struct tree_node* parent; 

	mcts_move current_move;

	int turns;

	tlptr childs_list;
}tree_node;

typedef struct thread_args{
	tree_node node;
	board board;
	double* result_winner;
	int* completed;

	int cur_moves;
} thread_args;

typedef struct wall{
	point point;
	orientation orient;
} wall;

typedef wall* wallptr;

#endif // STRUCTS_H