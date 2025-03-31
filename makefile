all: main

main: 
	gcc -o out main.c -lm

clear:
	rm out
