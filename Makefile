# Makefile

CC = gcc -fsanitize=address
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -std=c99 -O0 -g
LDFLAGS =
LDLIBS =

EXE := sukantsu
SRC := $(shell find src -name '*.c')
OBJ := ${SRC:.c=.o}
DEP := ${SRC:.c=.d}

all: ${EXE}

${EXE}: ${OBJ}
	${CC} ${CFLAGS} ${CPPFLAGS} ${LDFLAGS} ${OBJ} -o $@ ${LDLIBS}

clean:
	${RM} ${OBJ}
	${RM} ${DEP}
	${RM} ${EXE}

-include ${DEP}

# END
