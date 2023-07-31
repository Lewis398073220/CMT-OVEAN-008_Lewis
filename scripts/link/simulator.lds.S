
/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#define HEAP_SECTION_SIZE			0x10000

#undef  RAM_BASE
#undef  RAM_SIZE
#undef  SRAM_BASE
#undef  SRAM_SIZE

#undef  FLASH_BASE
#undef  FLASH_SIZE

#define RAM_BASE   0x31000000
#define RAM_SIZE   0x00400000

#define SRAM_BASE   0x01000000
#define SRAM_SIZE   0x200000

#define FLASH_BASE   0x00000000
#define FLASH_SIZE   0x00080000

#define SPI_FLASH_BASE   0x28000000
#define SPI_FLASH_SIZE   0x00800000

#define DDR_BASE   0x60000000
#define DDR_SIZE   0x80000000

#define VECTOR_SECTION_SIZE                     360

#define STACK_SECTION_SIZE              0x1000

/* Linker script to configure memory regions. */
MEMORY
{
	FLASH   	(rx) 	: ORIGIN = FLASH_BASE,   			LENGTH = FLASH_SIZE
	SPI_FLASH   (rx) 	: ORIGIN = SPI_FLASH_BASE,          LENGTH = SPI_FLASH_SIZE
	RAM    		(rwx) 	: ORIGIN = RAM_BASE,    			LENGTH = RAM_SIZE
	SRAM    	(rwx) 	: ORIGIN = SRAM_BASE,    	        LENGTH = SRAM_SIZE
}

/* Library configurations */
GROUP(libgcc.a)

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions ROM and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __export_fn_rom
 *   __exidx_start
 *   __exidx_end
 *   __rom_etext
 *   __rom_data_start__
 *   __rom_data_end__
 *   __rom_bss_start__
 *   __rom_bss_end__
 *   __HeapLimit
 *   __rom_StackLimit
 *   __rom_StackTop
 *   __rom_stack
 *   __Vectors_End
 *   __Vectors_Size
 *   __free_rom_ram
 *   __rom_end
 *   __free_rom
 *   __audio_const_rom_start
 *   __audio_const_rom_end
 *   __audio_const_rom_size
 */
ENTRY(Reset_Handler)

SECTIONS
{
    __text_region_start = ORIGIN(FLASH);
	.rom_text :
	{
        _vector_start = .;
        KEEP(*(.exc_vector_table))
        KEEP(*(".exc_vector_table.*"))

        KEEP(*(.vectors))

        _vector_end = .;

#ifndef NOSTD
		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
		*crtbegin.o(.dtors)
		*crtbegin?.o(.dtors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
		*(SORT(.dtors.*))
		*(.dtors)
#endif
		. = ALIGN(4);
        __etext = .;
	} > FLASH

	__exidx_start = .;
	.ARM.exidx (.) :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH
	__exidx_end = .;

	.ARM.extab (.) :
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH


	__rom_etext = LOADADDR(.rom_text) + SIZEOF(.rom_text);

	. = ALIGN(4);
	__rom_text0_end = .;

	.vector_table (ORIGIN(RAM)) (NOLOAD) :
	{
		__vector_table_start = .;
		KEEP(*(.vector_table))
		/* . = VECTOR_SECTION_SIZE; */
		. = ALIGN(4);
	} > RAM

	.data :
	{
		__rom_data_start__ = .;
        __data_start__ = .;
        __data_region_start = .;

		*(.data*)
		*(".data.*")
		*(.boot_data*)
		*(".kernel.*")
		. = ALIGN(4);

#ifndef NOSTD
		*(vtable)

		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);

		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
#endif

		/* All data end */
		__rom_data_end__ = .;
        __data_end__ = .;
	} > RAM AT > FLASH

	.bss (.) (NOLOAD) :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		__bss_start   = .;
		*(.bss*)
		*(.*_bss*)
		*(.boot_bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
		__bss_end   = .;
	} > RAM

	.heap (.) (NOLOAD) :
	{
		. = ALIGN(8);
		__HeapBase = .;
		__end__ = .;
		PROVIDE(end = __end__);
		. += HEAP_SECTION_SIZE;
		. = ALIGN(8);
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	/* .stack_dummy (.) (COPY) : */
	.stack_dummy (.) (NOLOAD) :
	{
		. = STACK_SECTION_SIZE;
		. = ALIGN(8);
	} > RAM

	.text ORIGIN(SPI_FLASH) :
	{
		*(.text*)
		*(.*_text*)
		. = ALIGN(4);
	} > SPI_FLASH

	.rom_rodata (.) :
	{
		*(.rodata*)
		*(.*_rodata*)

#ifndef NOSTD
		KEEP(*(.eh_frame*))
#endif
		*(.note.gnu.build-id)
		. = ALIGN(4);
	} > SPI_FLASH

	.build_info :
	{
		KEEP(*(.build_info))
		. = ALIGN(4);
        __etext = .;
	} > SPI_FLASH = 0x00000000

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
	__free_ram = __StackLimit - __HeapLimit;

	ASSERT((__vector_table_start & (1K - 1)) == 0, "vector table alignment error")
}

