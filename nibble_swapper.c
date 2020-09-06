/*******************************************************************************
*   Project: SMW Bruteforce Hacking
*
*   File: nibble_swapper.c
*
*   Description:

Swaps the High nibble and the Low nibble of a byte, following certain rules.
Rule 1) If both nibbles are equal, each nibble is increased by 0x08, applying
and "overflow" operation INDIVIDUALLY if the result is higher than 0xFF. 
Rule 2) If nibbles are different, the high nibble becomes the lower, 
and viceversa.

These rules allows to recover the initial value of the byte with the same
operation.

Example (all in hexadecimal, because it is easy to see):
Byte: 0xE3; after swap, it becomes 0x3E. Swap again, it becomes 0xE3 (original)
Byte: 0x00; after "swap", it becomes 0x88. "Swap" again, and it becomes 0x00
(because high nibble overflows to 0x"1"0, same as low nibble. But we ignore the
overflow. Think in this way:
Possible values: 0 1 2 3 4 5 6 7 8 9 A B C D E F
If the value is D, for example, and we add to it 0x03, we dont want a 2-digit
hexadecimal, we still want a 1-digit hexadecimal, so we consider that the
list of values are a "closed loop", so D becomes E, E becomes F, and F goes
back to 0.)
Byte 0x24; after "swap", it becomes 0xAC. "Swap" again, it becomes 0x24.

The byte to be swapped is a specific byte from a file. The file, as well as
the offset to the byte, are required parameters

Usage: nibble_swaper.exe <file> <offset>
i.e: nibble_swaper.exe raw_data.txt 3 // This changes the 4th byte (index 3)
of the file "raw_data.txt"

*
*   Notes: N/A
*
*   Contact: Alberto Martin Cajal, amartin.glimpse23<AT>gmail.com
*
*   URL: https://github.com/amcajal/SMW_Bruteforce_Hacking
*
*   License: GNU GPL v3.0
*
*   Copyright (C) 2020 Alberto Martin Cajal
*
*   This file is part of 8-Bit Hubble.
*
*   SMW Bruteforce Hacking is free software: 
*   you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   SMW Bruteforce Hacking is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************/

unsigned char swap_nibbles(unsigned char byte_to_swap)
{
    unsigned char new_value = 0x00;
    unsigned char high_nibble = 0x00;
    unsigned char low_nibble = 0x00;

    high_nibble = (byte_to_swap & 0xF0) >> 0x04;
    low_nibble = (byte_to_swap & 0x0F);

    if (high_nibble == low_nibble)
    {
        low_nibble = (low_nibble + 0x08) & 0x0F;
        high_nibble = low_nibble;        
    }

    new_value = ((new_value + low_nibble) << 4) + high_nibble;
    return new_value;
}


int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Usage: nibble_swaper.exe <file> <offset>\n");
        return 1;
    }

    FILE* file_worker = NULL;
    file_worker = fopen(argv[1], "r+");
    if (file_worker == NULL)
    {
        printf("Cannot open file %s\n", argv[1]);
        fclose(file_worker);
        return 1;
    }
   
    unsigned long offset = 0;
    offset = atol(argv[2]);

    rewind(file_worker);
    fseek(file_worker, offset, SEEK_SET);

    unsigned char byte_to_change = 0;
    fread(&byte_to_change, sizeof(unsigned char), 1, file_worker);
    unsigned char new_byte = swap_nibbles(byte_to_change);
    ungetc(byte_to_change, file_worker);    
    fwrite(&new_byte, sizeof(unsigned char), 1, file_worker);

    return 0;
}
