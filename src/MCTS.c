#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pthread.h>
#include <unistd.h>

#include "MCTS.h"

#define EVALUATION_THRESHOLD 0.8
#define NUMBER_OF_SIMS 50000
#define THREAD_NUMER 4
#define CONSTANT 1.4142135623730950488016887242096980785696718753769480731766797379907324784621

// Random Moves chance
#define WALL_VS_MOVE_CHANCE 0.5
#define BEST_VS_RANDOM_MOVE 0.8
#define BEST_WALLMOVE 0.5                   // this is much more expensive
#define GUIDED_RANDOM_WALL 0.75

int allDone(int* completed, int thread_num);

double MCTS(board *original_board,color player,listptr* adr, int* move_num)
{
	board b = boardcpy(*original_board);;
	tree_node *root = malloc(sizeof(tree_node)); 
	int cur_turns = list_length(adr);
	root->wins=0; root->passes=0; root->visited=YES; root->parent=NULL; root->childs_list=NULL; root->turns=cur_turns;
	
	if(player == WHITE)
		root->current_move.player=BLACK;
	else
		root->current_move.player=WHITE;
	
	int simulations=0;
	tree_node *PromisingNode;
	tree_node *nodeToExplore;
	tree_node *temp;
	//color rolloutResult;

	while(simulations < NUMBER_OF_SIMS)
	{
		simulations++;
		free_board(b);
		b = boardcpy(*original_board);

		//Phase 1 - Selection
		temp = findPromisingNode(root,&b);
		if(temp != NULL)
			PromisingNode = temp;

		//Phase 2 - Expand
		if(winner_helper(b) == NO_PLAYER && PromisingNode->visited == YES)
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
				PerformMove(nodeToExplore,&b);
			}	
		}
		nodeToExplore->visited=YES;

		int thread_num = THREAD_NUMER;
		thread_args* args;

		thread_args** arg_table = malloc(sizeof(thread_args*)*thread_num);
		double* winners = calloc(thread_num, sizeof(double));
		int* completed = calloc(thread_num, sizeof(int));
		pthread_t* threads = malloc(sizeof(pthread_t)*thread_num);

		for (int i=0; i < thread_num; i++)
		{
			args = malloc(sizeof(thread_args));
			args->board = boardcpy(b);
			args->node = *nodeToExplore;
			args->result_winner = &winners[i];
			args->completed = &completed[i];
			arg_table[i] = args;

			pthread_t thread_id;
			pthread_create(&thread_id, NULL, SimulateRollout, args);
			threads[i] = thread_id;
		}

		for (int i=0; i < thread_num; i++)
		{
			pthread_join(threads[i], NULL);
		}

		// while(!allDone(completed, thread_num))
		// {
		// 	sleep(0.01);
		// }

		double sum = 0;
		for (int i=0; i< thread_num; i++)
		{
			sum += winners[i];
		}	
		//rolloutResult = winners[0];

		for (int i=0; i < thread_num; i++)
		{
			//Destroy the pthreads
			//pthread_join(threads[i], NULL);
			free_board(arg_table[i]->board);
			free(arg_table[i]);
		}
		free(threads);
		free(arg_table);
		free(winners);
		free(completed);

		//Phase 4 - Update
		backPropagation(nodeToExplore,sum,player,thread_num);

		// if(simulations % 1000 == 0)
		// 	printf("		Simulation:%d		\n",simulations);

	}
	free_board(b);
	tree_node* BestNode = SelectBestChild(*root);

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

	PerformMove(BestNode,original_board);

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

int allDone(int* completed, int thread_num)
{
	for (int i=0; i < thread_num; i++)
	{
		if (completed[i] == 0)
			return 0;
	}
	return 1;
}

// Phase_1

tree_node* findPromisingNode(tree_node *root,board* b)
{
	tree_node *node = root;
	tlptr temp_ptr = node->childs_list;
	int n=0;
	

	while(temp_ptr != NULL)
	{
		n++;
		node = SelectBestChild(*node);
		PerformMove(node,b);
		temp_ptr = node->childs_list;
	}
	return node;
}

tree_node* SelectBestChild(tree_node node)
{
	//int parent_visits = node.passes;
	tlptr temp_ptr = node.childs_list;
	tree_node child;

	//int n=0;
	//int max_num=0;
	double maxUCT=0;
	//double cur_uct;
	tree_node* max_node;

	while(temp_ptr != NULL)
	{
		child = *(temp_ptr->child);

		double winrate = child.wins / ((double) child.passes);
		//printf("wins:%f, passes:%d, winrate:%f\n",child.wins,child.passes,winrate);

		if (!(node.current_move.player == BLACK))
			winrate = 1.0 - winrate;

		double uct;
		if (CONSTANT > 0) 
		{
			uct = winrate + CONSTANT * sqrt(log((double) NUMBER_OF_SIMS * THREAD_NUMER) / ((double) child.passes));
		} else 
		{
			uct = winrate;
		}
		//printf("UCT: %f, win: %f, passed:%d\n",uct,child.wins,child.passes);

		if (uct > maxUCT) 
		{
			
			maxUCT = uct;
			max_node = temp_ptr->child;
		}

		temp_ptr=temp_ptr->next;
	}

	return max_node;
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

void PerformMove(tree_node* node,board *b)
{
	player *p;
	if(node->current_move.player == WHITE)
		p = &b->player1;
	else
		p = &b->player2;	

	if(node->current_move.move == TRANSFER)
	{
		b->cells[p->position.y][p->position.x].player=NO_PLAYER;
		b->cells[node->current_move.point.y][node->current_move.point.x].player=p->player_color;
		p->position.x=node->current_move.point.x;
		p->position.y=node->current_move.point.y;		
	}
	else
	{
		p->available_walls--;
		b->cells[node->current_move.point.y][node->current_move.point.x].wall=node->current_move.orient;	
	}

	node->turns++;
}

//Phase_2

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
	child->turns = node->turns+1;

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

// Phase_3
double eval_position(board* board, color current_player)
{
	#define GUESS_WIN_CONF 0.95
    #define ROOM_FOR_ERROR 1 

	int white_walls = board->player1.available_walls;
	int black_walls = board->player2.available_walls;

	int white_path = find_min_steps(board,WHITE,board->player1.position,0);
	int black_path = find_min_steps(board,BLACK,board->player2.position,board->size - 1);

	// If one of the players has no walls left, and the other player has a sorter path to the end, the second player will win.
	if (black_walls <= 0 && white_path + (int)(current_player != WHITE) <= black_path - ROOM_FOR_ERROR) 
		return GUESS_WIN_CONF;

	if (white_walls <= 0 && black_path + (int)(current_player != BLACK) <= white_path - ROOM_FOR_ERROR) 
		return 1 - GUESS_WIN_CONF;

	if (black_walls <= 1 && white_walls >= 2 && white_path + (int)(current_player != WHITE) <= black_path - ROOM_FOR_ERROR) 
		return GUESS_WIN_CONF - 0.1;

	if (white_walls <= 1 && black_walls >= 2 && black_path + (int)(current_player != BLACK) <= white_path - ROOM_FOR_ERROR) 
		return 1 - (GUESS_WIN_CONF - 0.1);

	double wall_metric = 0.0;
	double max = white_walls > black_walls ? white_walls : black_walls;

	if (max > 0)
		wall_metric = ((white_walls > black_walls) ? +1 : -1) * (((double) pow(white_walls - black_walls, 2)) / ((double) pow(max, 2)));

	double path_diff = black_path - white_path + (current_player == WHITE ? +0.5 : -0.5); 
	double distance_metric = ((double) MAX(path_diff, 10.0)) / 10.0;

	return 0.5 + 0.2 * wall_metric + 0.2 * distance_metric; 
}

void* SimulateRollout(void* arg)
{
	thread_args* args = (thread_args*)arg;
	tree_node node = args->node;
	board board = args->board;
	int turns = node.turns;

	double board_winner = NO_PLAYER;
	color current_player = node.current_move.player;
	int heuristic_moves=0;
	bool no_wall_bool = NO;

	while (board_winner == NO_PLAYER && heuristic_moves < 40)
	{
		heuristic_moves++;
		current_player = ((current_player)%2)+1; //toggle player

		no_wall_bool = Semirandom_Move(current_player,&board,no_wall_bool,turns);
		turns++;

		// board_winner = winner_helper(board);
		if (winner_helper(board) != NO_PLAYER)
		{
			board_winner = winner_helper(board) == WHITE ? 1.0 : 0.0;
			break;
		}	

		board_winner = eval_position(&board,current_player);
	
		if (board_winner <= 1.0 - EVALUATION_THRESHOLD && board_winner >= EVALUATION_THRESHOLD) {
            break;
        }
	}

	// if(board_winner == NO_PLAYER)
	// {	
	// 	int eval = evaluate(board);
	// 	if(eval>0)
	// 		board_winner=WHITE;
	// 	else
	// 		board_winner=BLACK;	
	// }

	*(args->result_winner) = board_winner;
	*(args->completed) = YES;

	return arg;
}

void play_move(move move_type,point current,orientation orient,board* b,player* p)
{
	if(current.x != -1 && current.y!=-1)
	{
		if(move_type == TRANSFER)
		{
			b->cells[p->position.y][p->position.x].player=NO_PLAYER;
			b->cells[current.y][current.x].player=p->player_color;
			p->position.x=current.x;
			p->position.y=current.y;	
		}
		else
		{
			p->available_walls--;
			b->cells[current.y][current.x].wall=orient;	
		}
	}
}

bool Semirandom_Move(color cur_agent,board *b,bool no_wall,int turns)
{
	player agent, oponent;
	player *p;

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
	
	double wall_vs_move_prob = (turns <= 2) ? 0.0 :
                               (turns <= 6) ? (WALL_VS_MOVE_CHANCE / 2) : WALL_VS_MOVE_CHANCE;

	ListPtr wall_list = NULL;

	if (p->available_walls > 0 && no_wall == NO && ((double)rand() / RAND_MAX) < wall_vs_move_prob )
	{
		wall_list = GetAllPosibleWalls(b,cur_agent);

		if(ListSize(wall_list) == 0)
		{
			freeList(wall_list);
			return YES;
		}

		Pointer temp_array[ListSize(wall_list)]; 
		ListToArray(wall_list, temp_array);
		shuffle(temp_array, ListSize(wall_list));
		wallptr* wall_array = (wallptr*)temp_array;
		size_t  wall_array_size = ListSize(wall_list);

		if(((double)rand() / RAND_MAX) < BEST_WALLMOVE)
		{
			wallptr best_wallmove = NULL;
			int max_enc_diff = 0;
			int enemy_path = find_min_steps(b,oponent.player_color,oponent.position,oponent.player_color == WHITE ? b->size - 1 : 0);
			int our_path = find_min_steps(b,cur_agent,agent.position,cur_agent == WHITE ? b->size - 1 : 0);

			int index;
			for(index = 0; index< wall_array_size; index++)
			{
				wallptr wall = wall_array[index];

				// if (wall == NULL)
				// 	continue;

				while (wall == NULL)
				{
					index++;

					if (index >= wall_array_size)
						break;

					wall = wall_array[index];
				}

				if (!legal_wall(b, wall->point, wall->orient, NO))
				{
					wall_array[index] = NULL;
					continue;
				}

				int enemy_enc = enemy_path - find_min_steps(b,oponent.player_color,oponent.position,oponent.player_color == WHITE ? b->size - 1 : 0);
				
				if(enemy_path >= 0 && enemy_enc > 0)
				{
					int our_enc = our_path - find_min_steps(b,cur_agent,agent.position,cur_agent == WHITE ? b->size - 1 : 0);

					if (our_path >= 0 && enemy_enc > our_enc)
					{
						int enc_diff = our_enc - enemy_enc;
						if (enc_diff > max_enc_diff)
						{
							max_enc_diff = enc_diff;
							best_wallmove = wall;
						}
						else
						{
							wall_array[index] = NULL;
						}
					}
					else
					{
						wall_array[index] = NULL;
					}
				}
				else
				{
					wall_array[index] = NULL;
				}
				b->cells[wall->point.x][wall->point.y].wall = NO_WALL;
			}

			if (best_wallmove != NULL)
			{
				play_move(BUILD,best_wallmove->point,best_wallmove->orient,b,p);
				freeList(wall_list);
				return NO;
			}
		}
		else 
		{
			if (((double)rand() / RAND_MAX) < GUIDED_RANDOM_WALL)
			{
				wallptr wall_move = NULL;

				int enemy_path = find_min_steps(b,oponent.player_color,oponent.position,oponent.player_color == WHITE ? b->size - 1 : 0);
				int our_path = find_min_steps(b,cur_agent,agent.position,cur_agent == WHITE ? b->size - 1 : 0);

				for(int index = 0; index< wall_array_size; index++)
				{
					wallptr wall = wall_array[index];

					while (wall == NULL)
					{
						index++;

						if (index >= wall_array_size)
							break;

						wall = wall_array[index];
					}

					if (!legal_wall(b, wall->point, wall->orient, NO))
					{
						wall_array[index] = NULL;
						continue;
					}

					int enemy_enc = enemy_path - find_min_steps(b,oponent.player_color,oponent.position,oponent.player_color == WHITE ? b->size - 1 : 0);
					if (enemy_path >= 0 && enemy_enc > 0)
					{
						int our_enc = our_path - find_min_steps(b,cur_agent,agent.position,cur_agent == WHITE ? b->size - 1 : 0);
						if (our_path >= 0 && enemy_enc > our_enc)
						{
							wall_move = wall;
							break;
						}
					} 
					else
					{
						wall_array[index] = NULL;
					}
					b->cells[wall->point.x][wall->point.y].wall = NO_WALL;
				}	

				if (wall_move != NULL)
				{
					play_move(BUILD,wall_move->point,wall_move->orient,b,p);
					freeList(wall_list);
					return NO;
				}
			}
			else
			{
				wallptr wall_move = NULL;
				
				for(int index = 0; index< wall_array_size; index++)
				{
					wallptr wall = wall_array[index];

					while (wall == NULL)
					{
						index++;

						if (index >= wall_array_size)
							break;

						wall = wall_array[index];
					}

					if (!legal_wall(b, wall->point, wall->orient, YES))
					{
						wall_array[index] = NULL;
						continue;
					}

					wall_move = wall;
					break;
				}
				if (wall_move != NULL)
				{
					play_move(BUILD,wall_move->point,wall_move->orient,b,p);
					freeList(wall_list);
					return NO;
				}
			}
		}
	}

	if (wall_list != NULL)
		freeList(wall_list);

	//Play move
	point current;
	if(oponent.available_walls == 0 || ((double)rand() / RAND_MAX) < BEST_VS_RANDOM_MOVE)
	{
		current = FindShortestorLongestPath(b,cur_agent,SHORTEST);
		play_move(TRANSFER,current,NO_WALL,b,p);
		return NO;
	}
	else
	{
		ListPtr move_list = GetAllPosibleMoves(b,cur_agent);

		if(ListSize(move_list) == 0)
		{
			freeList(move_list);
			return YES;
		}

		Pointer temp_array[ListSize(move_list)]; 
		ListToArray(move_list, temp_array);
		point** move_array = (point**)temp_array;
		size_t  move_array_size = ListSize(move_list);

		srand(time(NULL));
		int r = rand() % move_array_size;

		current = *(move_array[r]);

		play_move(TRANSFER,current,NO_WALL,b,p);

		freeList(move_list);

		return NO;
	}
	return YES;
}

bool Heuristic_Move(color cur_agent,board *b,bool no_wall,int turns)
{
	player agent;//,oponent;
	player *p;
	point current;
	current.x=-1;current.y=-1;
	//point next_moves[5]={current,current,current,current,current};
	move move_type;
	orientation orient;

	if(cur_agent == WHITE)
	{
		agent = b->player1;
		//oponent = b->player2;
		p = &b->player1;
	}
	else
	{
		agent = b->player2;
		//oponent = b->player1;
		p = &b->player2;
	}

	// double wall_vs_move_prob = (turns <= 2) ? 0.0 :
    //                            (turns <= 6) ? (WALL_VS_MOVE_CHANCE / 2) : WALL_VS_MOVE_CHANCE;

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
			if(current.x == -1)
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
	return NO;	
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
	//player agent,oponent;
	pointptr H=NULL;
	pointptr V=NULL;
	int Hlen,Vlen;

	// if(cur_agent == WHITE)
	// {
	// 	agent = b->player1;
	// 	//oponent = b->player2;
	// }
	// else
	// {
	// 	agent = b->player2;
	// 	//oponent = b->player1;
	// }


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

void backPropagation(tree_node *node, double result,color player,int passes)
{
	color current_player = node->current_move.player;
	while(node != NULL)
	{
		node->passes+= passes;
		node->wins+= result;
		
		current_player = ((current_player)%2)+1;
		node=node->parent;
	}
}