LINT= -Wall -Wextra -pedantic -Wpadded -Fno-strict-aliasing -std=c99 -Werror
CFLAGS= -ffunction-sections -fdata-sections
LDFLAGS= -static -flto -Os -Wl,-gc-sections -s
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

all: test main

main: main.c ${FILES}
	${CC} ${LINT} ${CFLAGS} ${LDFLAGS} ${FILES} $< -o $@

test: \
	test.numbers \
	test.stack \
	test.pair \
	test.set \
	test.map \
	test.buffers \
	test.dump

test.%: tests/test.%
	$<

tests/test.%: tests/test-%.c ${FILES}
		${CC} ${LINT} ${CFLAGS} ${LDFLAGS} ${FILES} $< -o $@

clean:
	rm -f main
	rm -f tests/test.*
