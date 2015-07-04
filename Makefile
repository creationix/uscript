uscript: unix-main.c uscript.c uscript.h rpi-io.c
	clang -Wall -Werror -std=c99 -luv -o uscript unix-main.c

flash: unix-main.c arduino-main/arduino-main.ino
	/opt/arduino-1.6.5/arduino --preserve-temp-files --upload arduino-main/arduino-main.ino
	screen /dev/ttyACM0 9600

run: uscript
	./uscript

test:
	tcc -Wall -Werror -run test.c

clean:
	rm -f uscript
