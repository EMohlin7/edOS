ODIR = build/objects
ASMS_DIR = source/asm
CSRC_DIR = source/c

ASMS = stage2.s
CSRC = stage2.c

ASMO = stage2.o
	
COBJ = 

OBJS = ${ASMO} ${COBJ}

VPATH=source/c:source/asm:${ODIR}



build/stage2.bin: ${CSRC} ${ASMO}
	i686-elf-gcc -nostdlib -lgcc -mno-red-zone -o $@ $^ -T stage2.ld


${ODIR}/stage2.o: stage2.s
	mkdir -p ${ODIR}
	yasm -f elf32 $< -o $@

.PHONY: clean
clean:
	rm ${ODIR}/*