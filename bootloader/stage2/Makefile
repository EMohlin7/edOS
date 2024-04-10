BIN = build/stage2.bin

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -lgcc -mno-red-zone -fno-pie -DNULL=0 -Iinclude

ODIR = build/objects

ASMS_DIR = source/asm
ASMS = entry.s 
ASMO = entry.o 
	
CSRC_DIR = source/c
CSRC = pageTable.c
COBJ = pageTable.o longMode.o print.o display.o

OBJS = ${ASMO} ${COBJ}

VPATH=${CSRC_DIR}:${ASMS_DIR}:${ODIR}



${BIN}: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ -T stage2.ld


${ODIR}/entry.o: entry.s
	mkdir -p ${ODIR}
	yasm -f elf64 $< -o $@

${ODIR}/longMode.o: longMode.c
	mkdir -p ${ODIR}
	${CC} ${CFLAGS} -c -o $@ $^

${ODIR}/print.o: print.c
	mkdir -p ${ODIR}
	${CC} ${CFLAGS} -c -o $@ $^

${ODIR}/display.o: display.c
	mkdir -p ${ODIR}
	${CC} ${CFLAGS} -c -o $@ $^


${ODIR}/pageTable.o: pageTable.c
	mkdir -p ${ODIR}
	${CC} ${CFLAGS} -m32 -S -o temp.s $^
	echo '.code32' | cat - temp.s > temp && mv temp temp.s
	${CC} ${CFLAGS} -c -o $@ temp.s
	rm temp.s



.PHONY: clean
clean:
	rm ${ODIR}/*
	rm ${BIN}