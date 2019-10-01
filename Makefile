all: NixieClock

NixieClock: NixieClock.o
	gcc -o NixieClock NixieClock.o -lpthread


NixieClock.o: main.c
	gcc -o NixieClock.o -c main.c -ansi -pedantic -std=c11 -D_REENTRANT

clean:
	rm -rf *.o

mrproper: clean
	rm -rf NixieClock