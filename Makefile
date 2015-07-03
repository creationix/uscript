uscript: unix-main.c uscript.c rpi-io.c
	clang -Wall -Werror -std=c99 -g -luv -o uscript unix-main.c

run: uscript
	./uscript

test:
	tcc -Wall -Werror -run test.c

clean:
	rm -f uscript
