NAME=sudokurse
SRC=undo.c
SRC+=${NAME}.c
CC=clang

# Use this CC line instead to use GCC
#CC=gcc -std=c99

CFLAGS=-Weverything -pedantic

${NAME}: ${SRC}
	${CC} ${CFLAGS} ${SRC} -o ${NAME} -lncurses

clean:
	rm -f ${NAME}

.PHONY: clean

