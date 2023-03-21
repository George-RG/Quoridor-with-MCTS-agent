In this task, we tried to implement a C program that will
play with an opponent either a human, the user of the program, or
another program, through a controller / referee, the Quoridor game.The program has adopted the Quoridor Text Protocol for communication between programs that
play Quoridor: http://quoridor.di.uoa.gr/qtp/qtp.html.

Ιn more detail,

    Adminstrative Commands(made by sdi2100161)

    Setup Commands(made by sdi2100161)

    Core Play Commands
        playmove(made by sdi2100161):This command takes as input the color and the vertex of the move,checks if this move can be made through Quoridor game rules 
        (http://quoridor.di.uoa.gr/rules.html) and either makes the move or prints "illegal move" if the move is illegal.

        playwall(made by sdi2100118,sdi2100161):This command takes as input  the color,the vertex and the orientation of the wall,checks if the wall can be placed using a DFS
        path finding algorith and either places the wall or prints "illegal move" if the wall can't be placed.

        genmove(made by sdi2100118):This command takes as input  the color of the player for which to generate a move and using the monte-carlo tree search algorith
        (https://en.wikipedia.org/wiki/Monte_Carlo_tree_search),which performs 20000 simulations the engine makes a move or wall placement for the requested color.

        undo(made by sdi2100161,sdi2100118):This command takes as input  how many moves we want to undo and either performs the undo or prints "cannot undo" if the number the user gave is 
        greater than moves played.
    
    Tournament Commands(made by sdi2100161)

    Debug Commands(made by sdi2100118)


Our projects consists of c files(quoridor_dfs.c,quoridor_wallcheck.c,quoridor_helpful_commands.c,quoridor.c) and header files(quoridor_dfs.h,quoridor_wallcheck.h,quoridor_helpful_commands.h).
Τo complile them you have to perform:
    gcc -c <c file name>.c(for all c files)->creates a <c file name>.o program 
    gcc -o <quoridor> <c file name 1>.o <c file name 2>.o....(you have to include all the .o files) 

The task was implemented by:
Georgios Nikolaidis:sdi2100118
Ioanna Poulou:sdi2100161
