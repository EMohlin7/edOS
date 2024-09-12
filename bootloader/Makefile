BIN = build/bootloader.bin
ELF = build/bootloader.elf
SYM = build/bootloader.sym

${BIN}: bootloader.s#${ELF}
	yasm -f bin -o $@ $^
#	objcopy --only-keep-debug $^ ${SYM}
#	objcopy -O binary $^ $@

#${ELF}: build/bootloader.o
#	ld -o $@ $^ -T bootloader.ld

#build/bootloader.o: bootloader.s
#	mkdir -p build
#	yasm -f elf64 -g dwarf2 -o $@ $^

.PHONY: clean
clean:
	rm -r build/*