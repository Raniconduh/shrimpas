BIN=	shrimpas
SRC=	src/shrimpas.c src/parse.c src/opcode.c src/hashmap.c
OBJ=	${SRC:.c=.o}

CC?=	cc

CFLAGS+=	-Iinclude

all: ${BIN} include/*.h

${BIN}: ${OBJ}
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJ} -o $@

src/.c.o:
	${CC} -c ${CFLAGS} $<

clean:
	rm -f ${BIN} ${OBJ}

.PHONY: all clean
