all:quoridor_dfs quoridor_helpfull_commands quoridor_wallcheck dijkstra_s quoridor_all

quoridor_wallcheck:
	${CC} -c quoridor_wallcheck.c 
	
quoridor_helpfull_commands:
	${CC} -c quoridor_helpfull_commands.c 

quoridor_dfs:
	${CC} -c quoridor_dfs.c 

dijkstra_s:
	${CC} -c dijkstra_s.c 
	
quoridor_all:
	${CC} quoridor.c quoridor_dfs.o quoridor_helpfull_commands.o quoridor_wallcheck.o dijkstra_s.o -o quoridor  -lm
