OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
    . = 0x7e00;
    .text :
    {
        build/objects/asm/entry.o(.text)
        *(.text)
    }
    .data : {*(.data)}
    .bss : {*(.bss)}
}