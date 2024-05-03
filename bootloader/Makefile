BIN = build/bootloader.bin

${BIN}: bootloader.s
	mkdir -p build
	yasm -f bin -o $@ $^

.PHONY: clean
clean:
	rm ${BIN}