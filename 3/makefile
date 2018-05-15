CC = gcc

# Compile options
#   -ggdb   Bakes in debugging symbols for debugging with gdb
#   -Wall   Turns on verbose compilation warnings

CDEBUGFLAGS = -Wall -ggdb
NAME = mythread
CSUPPFLAGS = -w
MASTER_OBJ = socklib.o list.o ringmaster.o
MASTER_SRC = socklib.c list.c ringmaster.c
PLAYER_OBJ = socklib.o list.o player.o
PLAYER_SRC = socklib.c list.c player.c

all: master player

master: $(MASTER_OBJ) 
	@$(CC) $(CSUPPFLAGS) -o master $(MASTER_OBJ) 

player: $(PLAYER_OBJ)
	@$(CC) $(CSUPPFLAGS) -o player $(PLAYER_OBJ) 

debug: master-debug player-debug

master-debug:
	$(CC) $(CDEBUGFLAGS) -o master-debug $(MASTER_SRC) 

player-debug:
	$(CC) $(CDEBUGFLAGS) -o player-debug $(PLAYER_SRC) 

clean:
	rm master master-debug player player-debug a.out *.o
