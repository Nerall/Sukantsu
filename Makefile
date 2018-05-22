# Makefile

DEBUG_FLAGS = -O0 -g -rdynamic -fsanitize=address
RELEASE_FLAGS = -O2 -fshort-enums -DNGDEBUG

CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -std=c99 ${RELEASE_FLAGS}
LDLIBS = -lcsfml-system -lcsfml-network -lcsfml-graphics

EXE := sukantsu
SRC := $(wildcard src/*.c src/core/*.c src/AI/*.c src/network/*.c)
OBJ := ${SRC:.c=.o}
DEP := ${SRC:.c=.d}

%.o: %.c
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

all: pretty_message ${EXE}

${EXE}: ${OBJ}
	@echo "Linking: ${EXE}"
	@${CC} ${CFLAGS} ${OBJ} -o ${EXE} ${LDLIBS}

pretty_message:
	@echo "Target: \"${EXE}\""
	@echo "CC    : ${CC}"
	@echo "FLAGS : ${CPPFLAGS} ${CFLAGS}"
	@echo "LIBS  : ${LDLIBS}"
	@echo

clean:
	@echo "Removing .o files"
	@${RM} ${OBJ}

	@echo "Removing .d files"
	@${RM} ${DEP}

	@echo "Removing executable"
	@${RM} ${EXE}

	@echo

-include ${DEP}

# END
