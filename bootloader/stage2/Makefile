BIN = build/stage2.bin
ELF = build/stage2.elf
SYMS = build/stage2.sym

ASMC = yasm
ASM_FLAGS = -f elf64 -g dwarf2

CC = gcc
C_FLAGS = -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -lgcc -mno-red-zone -fpie -c -masm=intel -g -D'NULL=0' -iquote include

LINK = ld
LD_SCRIPT = stage2.ld

SRC_DIR = source

ASRC_NAMES = \
stage2.s \
loadFile.s \
pm.s \
lm.s

CSRC_NAMES = mapKernel.c

CSRC = $(addprefix ${SRC_DIR}/, $(CSRC_NAMES))
ASRC = $(addprefix ${SRC_DIR}/, ${ASRC_NAMES})

OBJ_DIR = build/objects
AOBJ_NAMES = $(ASRC_NAMES:.s=.o)
COBJ_NAMES = $(CSRC_NAMES:.c=.o)

COBJS = $(addprefix ${OBJ_DIR}/, ${COBJ_NAMES})
AOBJS = $(addprefix ${OBJ_DIR}/, ${AOBJ_NAMES})

OBJS = ${AOBJS} ${COBJS}

all: ${OBJ_DIR} ${BIN} 

${BIN}: ${ELF}
	objcopy --only-keep-debug $^ ${SYMS}
	objcopy -O binary $^ $@


${ELF}: ${OBJS}
	${LINK} -o $@ $^ -T ${LD_SCRIPT}

${AOBJS}: ${ASRC}
	${ASMC} ${ASM_FLAGS} -o $@ $(subst .o,.s, $(subst ${OBJ_DIR},${SRC_DIR}, $@))

${COBJS}: ${CSRC}
	${CC} ${C_FLAGS} -o $@ $(subst .o,.c, $(subst ${OBJ_DIR},${SRC_DIR}, $@))
	
${OBJ_DIR}:
	mkdir -p $@

.PHONY: clean
clean:
	rm -r build/*