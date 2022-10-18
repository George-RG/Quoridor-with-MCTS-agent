#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "MCTS.h"


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

// Phase_1

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
int SimulateRollout(tree_node node,board board)
{
	int board_winner = winner_helper(board);
	color current_player = node.current_move.player;
	int heuristic_moves=0;
	bool no_wall_bool = NO;

	while (board_winner == NO_PLAYER && heuristic_moves < 40)
	{
		heuristic_moves++;
		current_player = ((current_player)%2)+1; //toggle player

		no_wall_bool = Heuristic_Move(current_player,&board,no_wall_bool);
		//showboard(&board);
		board_winner = winner_helper(board);
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