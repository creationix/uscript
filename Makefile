LINT= -Wall -Wextra -pedantic -Wpadded -Fno-strict-aliasing -std=c99 -Werror
CFLAGS= -ffunction-sections -fdata-sections
LDFLAGS= -static -flto -O3 -Wl,-gc-sections -s
CC= musl-gcc

FILES= \
	src/buffers.c \
	src/dump.c \
	src/integer.c \
	src/map.c \
	src/pair.c \
	src/set.c \
	src/stack.c \
	src/uscript.c \
	src/utils.c

TESTS=\
	tests/test.numbers

main: main.c ${FILES}
	${CC} ${LINT} ${CFLAGS} ${LDFLAGS} ${FILES} $< -o $@

test: ${TESTS}
	tests/test.numbers

tests/test.%: tests/test-%.c ${FILES}
		${CC} ${LINT} ${CFLAGS} ${LDFLAGS} ${FILES} $< -o $@

clean:
	rm -f main
	rm -f tests/test.*
