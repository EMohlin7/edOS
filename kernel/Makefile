ELF = build/stage2.elf
SYMS = build/stage2.sym
BIN = build/stage2.bin

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -lgcc -mno-red-zone -fpie -c -masm=intel -g -D'NULL=0' -iquote include
ASMC = yasm
ASMFLAGS = -f elf64 -g dwarf2
LINK = ld
LD_SCRIPT = stage2.ld

ODIR = build/objects

ASMS_DIR = source/asm
ASMS = idt.s mem.s
ASMO_DIR = ${ODIR}/asm
ASMO := $(ASMS:.s=.o)

C32S_DIR = source/c
C32S =  
C32O_DIR = ${ODIR}/c32
C32O := $(C32S:.c=32.o)


CSRC_DIR = source/c
CSRC = \
longMode.c \
stdlib/print.c \
display/display.c \
system/sysInfo.c \
memMgmt/vmm.c \
memMgmt/pmm.c \
memMgmt/malloc.c \
system/sysTables/systemTables.c \
system/interrupts/interrupts.c \
system/interrupts/apic.c \
system/time/hpet.c \
stdlib/memdump.c \


COBJ_DIR = ${ODIR}/c
COBJ := $(CSRC:.c=.o)


OBJS = $(addprefix ${COBJ_DIR}/, ${COBJ}) $(addprefix ${C32O_DIR}/, ${C32O}) $(addprefix ${ASMO_DIR}/, ${ASMO}) 
OBJDIRS = $(dir ${OBJS})

all: ${OBJDIRS} ${BIN}
	


${BIN}: ${ELF}
	objcopy --only-keep-debug $^ ${SYMS}
	objcopy -O binary $^ $@


${ELF}: ${OBJS}
	${LINK} -o $@ $^ -T ${LD_SCRIPT}


#Assembly code
${ASMO_DIR}/%.o: ${ASMS_DIR}/%.s 
	${ASMC} ${ASMFLAGS} $^ -o $@


#32-bit c code
${C32O_DIR}/%32.o: ${C32S_DIR}/%.c
	${CC} ${CFLAGS} -m32 -DBIT32 -S -o $@_temp.s $^
	echo '.code32' | cat - $@_temp.s > $@_temp && mv $@_temp $@_temp.s
#	${CC} ${CFLAGS} -o $@ $@_temp.s
	-as -Z --gdwarf-2 -o $@ $@_temp.s
	rm $@_temp.s

#c code
${COBJ_DIR}/%.o: ${CSRC_DIR}/%.c
	${CC} ${CFLAGS} -o $@ $^


${OBJDIRS}:
	mkdir -p $@

.PHONY: clean
clean:
	rm build/* -r