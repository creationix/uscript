CFLAGS=-Os -Wall -Wextra -pedantic -Wpadded -Fno-strict-aliasing -std=c99 -Werror

vm: hash.c
	musl-gcc hash.c -o vm -static ${CFLAGS}

clean:
	rm -f vm
