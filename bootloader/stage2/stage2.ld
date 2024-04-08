OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x7e00;
    .text :
    {
        build/objects/entry.o(.text)
        *(.text)
    }
    .data : {*(.data)}
    .bss : {*(.bss)}
}