all: NixieClock

NixieClock: NixieClock.o 
	gcc -o NixieClock NixieClock.o 

NixieClock.o: main.c 
	gcc -o NixieClock.o -c main.c -W -Wall -ansi -pedantic

clean:
	rm -rf *.o

mrproper: clean
	rm -rf NixieClock