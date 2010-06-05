# Makefile for tron (C++)
VERSION=3.0.0
CFLAGS=-g -Wall -O


all: tron

tron: tron.o
	g++ ${CFLAGS} -lpthread -o tron tron.o

tron.o: tron.cpp lightcycle.h
	g++ ${CFLAGS} -c -lpthread tron.cpp

clean:
	rm -f ./tron
	rm -f ./tron.o
	rm -f ./tron.log

check: tron
	./tron -x 15 -y 8 0 0 typ1 3 9 typ1
	
dist: all distclean
	@mkdir ../tron-${VERSION}/
	@cp * ../tron-${VERSION}/
	@tar cvzf ./tron-${VERSION}.tgz ../tron-${VERSION}
	@rm -r ../tron-${VERSION}/

distclean:
	@rm -f ./tron-${VERSION}.tgz
	@rm -f ./*~
	@rm -f ./tron.o
	@rm -f ./tron.log

