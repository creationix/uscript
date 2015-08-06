test: uscript.c uscript.h test.c
	$(CC) -Wall -Werror -std=c99 -o test test.c
	./test && rm test

uscript: main.c uscript.c rpi-io.c
	$(CC) -Wall -Werror -std=gnu99 -o uscript main.c

run: uscript
	sudo ./uscript
