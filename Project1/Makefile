

all: pmatch1 pmatch2 pmatch3

pmatch1: pmatch1.c
	gcc -g -Wall -o pmatch1 pmatch1.c

pmatch2: pmatch2.c
	gcc -g -Wall -o pmatch2 pmatch2.c

pmatch3: pmatch3.c
	gcc -g -Wall -o pmatch3 pmatch3.c -lpthread

clean:
	rm -fr *~ *.o pmatch1 pmatch2 pmatch3
