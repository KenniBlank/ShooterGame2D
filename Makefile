build:
	gcc -g mainGame.c -o game -Wall -lSDL2 -lSDL2_image -std=c99
	./game

clean:
	rm ./game
