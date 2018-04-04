# Makefile

DEBUG_FLAGS = -O0 -g -rdynamic -fsanitize=address
RELEASE_FLAGS = -O2 -fshort-enums -DNGDEBUG

CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -std=c99 ${RELEASE_FLAGS}
LDLIBS = -lcsfml-network

EXE := sukantsu
SRC := $(wildcard src/*.c src/core/*.c src/AI/*.c src/network/*.c)
OBJ := ${SRC:.c=.o}
DEP := ${SRC:.c=.d}

all: ${EXE}

${EXE}: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o ${EXE} ${LDLIBS}

clean:
	${RM} ${OBJ}
	${RM} ${DEP}
	${RM} ${EXE}

-include ${DEP}

# END
