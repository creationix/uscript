uscript: unix-main.c uscript.c rpi-io.c
	clang -Wall -Werror -std=c99 -luv -o uscript unix-main.c

flash: unix-main.c arduino-main/arduino-main.ino
	/opt/arduino-1.6.6/arduino --preserve-temp-files --upload arduino-main/arduino-main.ino && screen /dev/ttyUSB0 9600

run: uscript
	./uscript

test:
	clang -Wall -Werror -std=c99 test.c
	./a.out
	rm a.out
	#tcc -Wall -Werror -run test.c

clean:
	rm -f uscript
