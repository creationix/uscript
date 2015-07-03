uscript: unix-main.c uscript.c rpi-io.c
	clang -Wall -Werror -std=c99 -luv -o uscript unix-main.c

run: uscript
	rlwrap uscript

test:
	tcc -Wall -Werror -run test.c

clean:
	rm -f uscript
