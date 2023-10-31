#include "stdint.h"
#include "stdio.h"

// Entry point for the second-stage bootloader C code.
// It is called by the Main.asm code (which is the entry point of the 2nd stage bootloader)
void _cdecl cstart_(uint16_t bootDrive){
	puts("Loading bootloader stage 2...\r\n");

	printf("Testing printf: %% %c %s\r\n", 'a', "my_string");
	printf("Testing printf (integers): %d %i %x %p %o %hd %hi %hhu %hhd\r\n", 1234, -5678, 0x7fff, 0xbeef, 012345, (short)57, (short)-42, (unsigned char) 20, (char)-10);
	printf("Testing printf (longs): %ld %lx %lld %llx\r\n", -100000000l, 0x7ffffffful, 10200300400ll, 0xeeeeaaaa7777ffffull);

	for(;;);
}
