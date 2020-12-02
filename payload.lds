SECTIONS {

	.text 0x100000 : {
		__payload = .;

		__text = .;
		*(.text.entry)
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		__text_end = .;

		. = ALIGN(8);

		__data = .;
		*(.data)
		*(.data.*)
		__data_end = .;

		. = ALIGN(8);

		__bss = .;
		*(.bss)
		*(.bss.*)
		__bss_end = .;

		__payload_end = .;
	}
}
