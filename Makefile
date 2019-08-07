CC=gcc
LDFLAGS=-lncurses

.PHONY=all
all: game

.PHONY=clean
clean:
	$(RM) *~ *.o snake

OBJECTS=snake_game.o

game: $(OBJECTS)
	$(CC) $(OBJECTS) -o snake $(LDFLAGS)
