ELF = build/stage2.elf
SYMS = build/stage2.sym
BIN = build/stage2.bin

CC = gcc
LINK = ld
CFLAGS = -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -lgcc -mno-red-zone -fno-pie -c -g -DNULL=0 -Iinclude

ODIR = build/objects

ASMS_DIR = source/asm
ASMS = entry.s 
ASMO = entry.o 
	
CSRC_DIR = source/c
CSRC = pageTable.c
COBJ = longMode.o print.o display.o pageTable.o  

OBJS = ${COBJ} ${ASMO} 

VPATH=${CSRC_DIR}:${ASMS_DIR}:${ODIR}

${BIN}: ${ELF}
	objcopy --only-keep-debug $^ ${SYMS}
	objcopy -O binary $^ $@


${ELF}: ${OBJS}
	${LINK} -o $@ $^ -T stage2.ld


${ODIR}/entry.o: entry.s
	mkdir -p ${ODIR}
	yasm -f elf64 -g dwarf2 $< -o $@

${ODIR}/longMode.o: longMode.c
	mkdir -p ${ODIR}
	${CC} ${CFLAGS} -o $@ $^

${ODIR}/print.o: print.c
	mkdir -p ${ODIR}
	${CC} ${CFLAGS} -o $@ $^

${ODIR}/display.o: display.c
	mkdir -p ${ODIR}
	${CC} ${CFLAGS} -o $@ $^


${ODIR}/pageTable.o: pageTable.c
	mkdir -p ${ODIR}
	${CC} ${CFLAGS} -m32 -S -o temp.s $^
	echo '.code32' | cat - temp.s > temp && mv temp temp.s
	${CC} ${CFLAGS} -o $@ temp.s
	rm temp.s



.PHONY: clean
clean:
	rm build/* -r