CFLAGS= -Wall -Wextra -pedantic -Wpadded -Fno-strict-aliasing -std=c99 -Werror -flto -ffunction-sections -fdata-sections
LDFLAGS= -static -flto -Os -Wl,-gc-sections -s

vm: hash.c
	musl-gcc hash.c -o vm -static ${CFLAGS} ${LDFLAGS}

clean:
	rm -f vm
