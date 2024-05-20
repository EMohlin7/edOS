ELF = build/stage2.elf
SYMS = build/stage2.sym
BIN = build/stage2.bin

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -lgcc -mno-red-zone -fno-pie -c -masm=intel -g -DNULL=0 -Iinclude
ASMC = yasm
ASMFLAGS = -f elf64 -g dwarf2
LINK = ld
LD_SCRIPT = stage2.ld

ODIR = build/objects

ASMS_DIR = source/asm
ASMS = entry.s idt.s mem.s
ASMO_DIR = ${ODIR}/asm
ASMO := $(ASMS:.s=.o)

C32S_DIR = source/c
C32S = pageTable.c
C32O_DIR = ${ODIR}/c32
C32O := $(C32S:.c=.o)


CSRC_DIR = source/c
CSRC = longMode.c print.c display.c stdlib.c interrupts.c sysInfo.c vmm.c
COBJ_DIR = ${ODIR}/c
COBJ := $(CSRC:.c=.o)


OBJS = $(addprefix ${COBJ_DIR}/, ${COBJ}) $(addprefix ${C32O_DIR}/, ${C32O}) $(addprefix ${ASMO_DIR}/, ${ASMO}) 


${BIN}: ${ELF}
	objcopy --only-keep-debug $^ ${SYMS}
	objcopy -O binary $^ $@


${ELF}: ${OBJS}
	${LINK} -o $@ $^ -T ${LD_SCRIPT}


#Assembly code
${ASMO_DIR}/%.o: ${ASMS_DIR}/%.s
	mkdir -p ${ASMO_DIR}
	${ASMC} ${ASMFLAGS} $^ -o $@


#32-bit c code
${C32O_DIR}/%.o: ${C32S_DIR}/%.c
	mkdir -p ${C32O_DIR}
	${CC} ${CFLAGS} -m32 -S -o $@_temp.s $^
	echo '.code32' | cat - $@_temp.s > $@_temp && mv $@_temp $@_temp.s
	${CC} ${CFLAGS} -o $@ $@_temp.s
	rm $@_temp.s

#c code
${COBJ_DIR}/%.o: ${CSRC_DIR}/%.c
	mkdir -p ${COBJ_DIR}
	${CC} ${CFLAGS} -o $@ $^


.PHONY: clean
clean:
	rm build/* -r