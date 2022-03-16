int** create2DIntArray(int rows,int cols,int value);
void free2DArray(void** A ,int rows);
point FindPathToGoal(board b,color agent, int** return_dist, point** return_prev);
point FindShortestorLongestPath(board *b, color agent,distance dist);