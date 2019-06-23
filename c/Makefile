# Author: Evan Shipman
# Title : ViHash Makefile

EXE = vihash
FLAGS = -ansi -pedantic -Wall -O3
LIBS = -lssl -lcrypto

all:
	gcc ${FLAGS} vihash.c ${LIBS} -o ${EXE}

clean:
	rm -rf ${EXE}
