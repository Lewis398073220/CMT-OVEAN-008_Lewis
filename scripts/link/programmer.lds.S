/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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

#include "plat_addr_map.h"

#ifdef NO_TRUSTZONE
#define RAMRUN_SEC_ADDR(a)      	RAM_NS_TO_S(a)
#else
#define RAMRUN_SEC_ADDR(a)      	(a)
#endif

#ifdef ROM_IN_FLASH
#define MIN_BURN_BUFFER_SIZE		0x12000
#else
#define MIN_BURN_BUFFER_SIZE		0x2000
#endif

#define HEAP_SECTION_SIZE			0x40
#define STACK_SECTION_SIZE			0x1000

#define CODE_MSG_OVERHEAD			8

#ifdef PROGRAMMER_LOAD_SIMU
#undef RAM_SIZE
#ifdef ROM_IN_FLASH
#define RAM_SIZE					0x18000
#else
#define RAM_SIZE					0x8000
#endif
#endif

#ifdef MAIN_RAM_USE_TCM
#define RAM							ITCM
#define RAMX						ITCM
#endif

#if !(defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE))
#define LOAD_RAM					RAM
#endif

#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
#define CP_STACK_SECTION_SIZE		0x1000
#endif

/* Linker script to configure memory regions. */
MEMORY
{
	ROM    		(rx)	: ORIGIN = ROM_BASE,			LENGTH = ROM_SIZE	/* see plat_addr_map.h and common.mk */
#ifdef MAIN_RAM_USE_TCM
	DTCM   		(rw)	: ORIGIN = RAM_BASE,			LENGTH = RAM_SIZE
	ITCM   		(rwx)	: ORIGIN = RAMX_BASE,			LENGTH = RAMX_SIZE
#else
	RAM    		(rwx)	: ORIGIN = RAM_BASE,			LENGTH = RAM_SIZE
	RAMX   		(rx)	: ORIGIN = RAMX_BASE,			LENGTH = RAM_SIZE
#endif
#if defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE)
	LOAD_RAM	(rwx)	: ORIGIN = LOAD_RAM_BASE,		LENGTH = LOAD_RAM_SIZE
#endif
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
	RAMCP 		(rwx)	: ORIGIN = RAMCP_BASE,				LENGTH = RAMCP_SIZE
	RAMCPX 		(rwx)	: ORIGIN = RAMCPX_BASE,				LENGTH = RAMCPX_SIZE
#endif
#ifdef MEM_POOL_BASE
    MEM_POOL    (rwx)   : ORIGIN = MEM_POOL_BASE,           LENGTH = MEM_POOL_SIZE
#endif
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
 *   __exec_struct_start
 *   __exec_load_start
 *   __exec_load_end
 *   __cust_cmd_init_tbl_start
 *   __cust_cmd_init_tbl_end
 *   __cust_cmd_hldr_tbl_start
 *   __cust_cmd_hldr_tbl_end
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __free_ram
 */
ENTRY(programmer_start)

SECTIONS
{
	__export_fn_rom = (ORIGIN(ROM) + LENGTH(ROM) - ROM_BUILD_INFO_SECTION_SIZE - ROM_EXPORT_FN_SECTION_SIZE);

	.vector_table (ORIGIN(RAM)) (NOLOAD) :
	{
		__vector_table_start = .;
		KEEP(*(.vector_table))
		. = VECTOR_SECTION_SIZE;
		. = ALIGN(4);
	} > RAM

	.reboot_param (NOLOAD) :
	{
		KEEP(*(.reboot_param))
		. = REBOOT_PARAM_SECTION_SIZE;
		. = ALIGN(4);
	} > RAM

	.burn_buffer (NOLOAD) :
	{
		/* The size of .burn_buffer should be greater than __rom_HeapLimit. In most cases 8K is enough. */
		*(.burn_buffer)
		. = MAX(., MIN_BURN_BUFFER_SIZE);
		. = ALIGN(4);
	} > LOAD_RAM

/*
	.code_dummy (NOLOAD) :
	{
		. = CODE_MSG_OVERHEAD;
		. = ALIGN(4);
	} > LOAD_RAM
*/

	.boot_struct :
	{
		__boot_struct_start__ = .;
		KEEP(*(.boot_struct))
		__boot_struct_end__ = .;

		__exec_struct_start = .;
		KEEP(*(.exec_struct))
		. = ALIGN(4);
	} > LOAD_RAM

#if defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE)
	.pgm_boot_text :
	{
		*(.pgm_start)
		. = ALIGN(4);
	} > LOAD_RAM
#endif

	.got_info :
	{
		__got_info_start = .;
		__got_plt_start = .;
		*(.got.plt)
		__got_plt_end = .;
		__got_start = .;
		*(.got)
		__got_end = .;
		. = ALIGN(4);
	} > LOAD_RAM

	.rel.plt :
	{
		__rel_plt_start = .;
		*(.rel.plt)
		__rel_plt_end = .;
	} > LOAD_RAM

	.rel.dyn :
	{
		__rel_dyn_start = .;
		*(.rel.data.rel.ro*)
		*(.rel.gnu.linkonce.d.rel.ro*)
		*(.rel.*)
		__rel_dyn_end = .;
	} > LOAD_RAM

	.data.rel.ro :
	{
		__rodata_rel_start = .;
		*(.data.rel.ro.local*)
		*(.data.rel.ro*)
		__rodata_rel_end = .;
	} > LOAD_RAM

	__exec_load_start = .;

	.dummy_exec_data_start (NOLOAD) :
	{
		__exec_data_start = .;
	} > RAM

	. = RAM_TO_RAMX(.);

	.text (.) : AT (__exec_load_start)
	{
		__text_start__ = .;
		*(.text*)

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
	} > RAMX

	.ARM.extab (.) : AT (LOADADDR(.text) + SIZEOF(.text))
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > RAMX

	__exidx_start = .;
	/* .ARM.exidx contains R_ARM_PREL31 (+-0x40000000) offset to functions, which means
	 * the session location cannot be too far away from the function addresses */
	.ARM.exidx (.) : AT (LOADADDR(.ARM.extab) + SIZEOF(.ARM.extab))
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > RAMX
	__exidx_end = .;

	. = RAMX_TO_RAM(.);

	.rodata (.) : AT (LOADADDR(.ARM.exidx) + SIZEOF(.ARM.exidx))
	{
		__rodata_start__ = .;
		*(.rodata*)

		. = ALIGN(4);
		__cust_cmd_init_tbl_start = .;
		KEEP(*(.cust_cmd_init_tbl))
		__cust_cmd_init_tbl_end = .;

		. = ALIGN(4);
		__cust_cmd_hldr_tbl_start = .;
		KEEP(*(.cust_cmd_hldr_tbl))
		__cust_cmd_hldr_tbl_end = .;

#ifndef NOSTD
		KEEP(*(.eh_frame*))
#endif
		*(.note.gnu.build-id)
		. = ALIGN(4);
	} > RAM

#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
	__cp_text_sram_start_flash__ = LOADADDR(.rodata) + SIZEOF(.rodata);

	.cp_text_sram (ORIGIN(RAMCPX)) : AT (__cp_text_sram_start_flash__)
	{
		__cp_text_sram_exec_start__ = .;
		*(.cp_text_sram*)
		*:cp_queue.o(.text*)
		. = ALIGN(4);
		__cp_text_sram_exec_end__ = .;
	} > RAMCPX

	__cp_text_sram_start = RAMX_TO_RAM(__cp_text_sram_exec_start__);
	__cp_text_sram_end = RAMX_TO_RAM(__cp_text_sram_exec_end__);

	.vector_table_cp (ORIGIN(RAMCP)) (NOLOAD) :
	{
		KEEP(*(.vector_table_cp))
		. = VECTOR_SECTION_SIZE;
		. = ALIGN(4);
	} > RAMCP

	__cp_data_sram_start_flash__ = __cp_text_sram_start_flash__ + SIZEOF(.cp_text_sram);

	.cp_data_sram : AT (__cp_data_sram_start_flash__)
	{
		__cp_data_sram_start = .;
		*(.cp_data*)
		*:cp_queue.o(.data* .rodata*)
		. = ALIGN(4);
		__cp_data_sram_end = .;
	} > RAMCP

	__cp_sram_end_flash__ = __cp_data_sram_start_flash__ + SIZEOF(.cp_data_sram);

	.cp_bss_sram (NOLOAD) : AT (ADDR(.cp_bss_sram))
	{
		__cp_bss_sram_start = .;
		*(.cp_bss*)
		. = ALIGN(4);
		__cp_bss_sram_end = .;
	} > RAMCP

	__cp_stack_top = ORIGIN(RAMCP) + LENGTH(RAMCP);
	__cp_stack_limit = __cp_stack_top - CP_STACK_SECTION_SIZE;

	__etext = __cp_sram_end_flash__;
#if !(defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE))
	. = __etext;
#endif
#else
	__etext = LOADADDR(.rodata) + SIZEOF(.rodata);
#endif

	.data (.) : AT (__etext)
	{
		__data_start__ = .;
		*(.data*)
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
		__data_end__ = .;
	} > RAM

	.build_info (.) : AT (LOADADDR(.data) + SIZEOF(.data))
	{
		KEEP(*(.build_info))
		. = ALIGN(4);
	} > RAM = 0x00000000

	__boot_struct_size__ = __boot_struct_end__ - __boot_struct_start__;
	/* The following section be the last loaded section */
	__exec_load_end = LOADADDR(.build_info) + SIZEOF(.build_info);

	.code_start_addr (__exec_load_end) :
	{
#ifdef MTEST_BUILTIN
		LONG(__boot_struct_end__ - __boot_struct_start__);
#endif
		LONG(RAMRUN_SEC_ADDR(ABSOLUTE(__exec_struct_start)));
	} > LOAD_RAM

	.bss (NOLOAD) :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM

	.heap (NOLOAD) :
	{
		. = ALIGN(8);
		__HeapBase = .;
		__end__ = .;
		end = __end__;
		. += HEAP_SECTION_SIZE;
		. = ALIGN(8);
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (COPY) :
	{
		. = STACK_SECTION_SIZE;
		. = ALIGN(8);
	} > RAM

	/DISCARD/ :
	{
		*(.interp)
		*(.hash)
		*(.dynsym)
		*(.dynstr)
		*(.gnu.version)
		*(.gnu.version_d)
		*(.gnu.version_r)
		*(.rela.plt)
		*(.rela.data.rel.ro*)
		*(.rela.gnu.linkonce.d.rel.ro*)
		*(.rela.*)
		*(.plt)
		*(.dynamic)
		*(.iplt*)
		*(.igot*)
	}

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
	__free_ram = __StackLimit - __HeapLimit;

#ifdef MEM_POOL_BASE
	.mem_pool (ORIGIN(MEM_POOL)) (NOLOAD) :
	{
		__mem_pool_start__ = .;
		. = LENGTH(MEM_POOL);
		__mem_pool_end__ = .;
	} > MEM_POOL
#elif !(defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0) && defined(MEM_POOL_IN_CP_RAM))
	__mem_pool_start__ = __HeapLimit;
	__mem_pool_end__ = __StackLimit;
#endif

	ASSERT((__vector_table_start & (1K - 1)) == 0, "vector table alignment error")
}
