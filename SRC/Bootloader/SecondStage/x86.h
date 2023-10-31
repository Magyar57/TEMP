#ifndef __X86_H__
#define __X86_H__

#include "stdint.h"

// Note: the label in the assembly file has an '_' in front of the function name.
// It is because of the cdecl convention, which stipulates that compiled assembly function labels must have an underscore prepended to the function name.

void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* remainderOut);

// Write a character c to the screen
void _cdecl x86_Video_WriteCharTeletype(char c, uint8_t page);

#endif
