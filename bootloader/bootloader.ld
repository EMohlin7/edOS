OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
    . = 0x7c5a;
    .text :
    {
        start = .;
        build/bootloader.o(.text)
        *(.text)
    }
    //.data : {*(.data)}
    //.bss : {*(.bss)}
}