test:
	make -C lib test

flash:
	make -C arduino flash

run:
	make -C unix run

clean:
	git clean -dfx
