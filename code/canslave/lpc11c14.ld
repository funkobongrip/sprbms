ENTRY(Reset_Handler)

MEMORY
{
    flash   :   ORIGIN = 0x00000000, LENGTH = 0x8000
    sram    :   ORIGIN = 0x100000C0, LENGTH = (0x1F20 - (150 * 4))
}

_end_stack = 0x10002000;

SECTIONS {
    . = ORIGIN(flash);

    vectors :
    {
        *(.vectors)
    } >flash

    .text :
    {
	. = ALIGN(4);
        _data_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        _data_table_end = .;
        *(.text)
        *(.rodata)
        *(.rodata*)
        _end_text = .;
    } >flash

    .data :
    {
	. = ALIGN(4);
        _start_data = .;
        *(.data)
        *(.data*)
        _end_data = .;
    } >sram AT >flash

    .bss :
    {
	. = ALIGN(4);
        _start_bss = .;
        *(.bss)
        *(.bss*)
        _end_bss = .;
    } >sram

    . = ALIGN(4);

    _start_stack = .;

}

_end = .;
PROVIDE(end = .);
