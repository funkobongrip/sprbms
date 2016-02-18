//*****************************************************************************
//   +--+       
//   | ++----+   
//   +-++    |  
//     |     |  
//   +-+--+  |   
//   | +--+--+  
//   +----+    Copyright (c) 2010 Code Red Technologies Ltd.
//
// Corresponding linker script changes were implemented in v3.6 and are ignored
// if used with previous releases.
//
//*****************************************************************************

#ifndef CR_SECTION_MACROS_H_INCLUDED
#define CR_SECTION_MACROS_H_INCLUDED

// A macro for placing text (code), data, or bss into a named RAM section
// These will be automatically placed into the named section by the linker.
//
// RAM banks are numbered starting from 1. The actual configuration is 
// dependent on the selected MCU type.
//
// Example:
//        __SECTION(data,RAM1) char buffer[1024] ;
//
// This will place the 1024 byte buffer into the RAM1
//
#define __SECTION_EXT(type, bank, name) __attribute__ ((section("." #type ".$" #bank "." #name)))
#define __SECTION(type, bank) __attribute__ ((section("." #type ".$" #bank)))
#define __SECTION_SIMPLE(type) __attribute__ ((section("." #type)))

#define __DATA_EXT(bank, name) __SECTION_EXT(data, bank, name)
#define __DATA(bank) __SECTION(data, bank)

#define __BSS_EXT(bank, name) __SECTION_EXT(bss, bank, name)
#define __BSS(bank) __SECTION(bss, bank)

// Macros for placing text (code), data, or bss into a section that is automatically
// placed after the vectors in the target image.
#define __AFTER_VECTORS_EXT(name) __attribute__ ((section(".after_vectors.$" #name)))
#define __AFTER_VECTORS __attribute__ ((section(".after_vectors")))

// Macros for placing data or bss into a section that has the NOLOAD option set in the linker script
#define __NOINIT_DEF __SECTION_SIMPLE(noinit)
#define __NOINIT_EXT(bank, name) __SECTION_EXT(noinit, bank, name)
#define __NOINIT(bank) __SECTION(noinit, bank)

#endif /* CR_SECTION_MACROS_H_INCLUDED */
