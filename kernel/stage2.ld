OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
    . = 0xffffff0000000000;
    .text :
    {
        build/objects/c/longMode.o(.text)
        *(.text)
    }
    .data : {*(.data)}
    .bss : {*(.bss)}
}