OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
    . = 0x7e00;
    .text :
    {
        build/objects/stage2.o(.text)
        build/objects/loadFile.o(.text)
        build/objects/pm.o(.text)
        *(.text)
    }
    .data : {*(.data)}
    .bss : {*(.bss)}
}