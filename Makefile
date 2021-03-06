NAME=sudokurse
SRC=src/undo.c
SRC+=src/${NAME}.c
CC=clang

# Use this CC line instead to use GCC
#CC=gcc -std=c99

CFLAGS=-Weverything -pedantic

${NAME}: ${SRC}
	${CC} ${CFLAGS} ${SRC} -o ${NAME} -lncurses

clean:
	rm -f ${NAME}

install:
	install -D ${NAME} ${DESTDIR}/usr/bin/${NAME}

.PHONY: clean install

