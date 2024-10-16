CFLAGS = -Wall -g -std=c99
LIBS   = -lSDL2 -lSDL2_image -lSDL2_ttf
SOURCE = mainGame.c

ifeq ($(OS),Windows_NT)
	GAME = "game.exe"
	CFLAGS += -L ./
else
	GAME = "game"
endif

all:
	gcc $(SOURCE) -o $(GAME) $(CFLAGS) $(LIBS)

clean:
	rm ./$(GAME)
