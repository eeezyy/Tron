# Makefile for tron
VERSION=2.0.0
CFLAGS=-g -Wall -O


all: tron

tron: tron.o
	gcc ${CFLAGS} -lpthread -o tron tron.o

tron.o: tron.c
	gcc ${CFLAGS} -c -lpthread tron.c

clean:
	rm -f ./tron
	rm -f ./tron.o
	
dist: all distclean
	mkdir ../tron-${VERSION}/
	cp * ../tron-${VERSION}/
	tar cvzf ./tron-${VERSION}.tgz ../tron-${VERSION}
	rm -r ../tron-${VERSION}/

distclean:
	rm -f ./tron-${VERSION}.tgz
	rm -f ./*~
	rm -f ./tron.o

