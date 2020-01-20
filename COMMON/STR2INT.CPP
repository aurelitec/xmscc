/*------------------------------------------------------------------------*/
/*                                                                        */
/*  String to integer conversion routines                                 */
/*                                                                        */
/*  STR2INT.CPP: C++ Source Code                                          */
/*                                                                        */
/*  Version 1.0 (Jul 16 1997)                                             */
/*  Copyright (c) 1997 Devliber (https://devliber.com)                    */
/*                                                                        */
/*  Written and compiled in Borland C++ 3.1.                              */
/*                                                                        */
/*  Licensed under the MIT License. See LICENSE file in the project       */
/*  root for full license information.                                    */
/*                                                                        */
/*------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include "common\str2int.h"

/* ----------------------------------------------------------------------- *

  Name		strxchr

  Declaration	char *strxchr(const char *s, int c)

  Description	Scans a string for the first occurence of a character that
		is not equal to a given character.

  Return value	� On success, returns a pointer to the first occurrence of
		any other character than c in string s.
		� On error (if only c occurs in s), returns null.

 * ----------------------------------------------------------------------- */

char *strxchr(const char *s, int c)
{
  asm	mov	bx, s

Next:
  asm	mov	al, [bx]
  asm	xor	ah, ah
  asm	or	ax, ax
  asm	jz	EOS
  asm	inc	bx
  asm	cmp	ax, c
  asm	je	Next

  asm	mov	ax, bx
  asm	dec	ax
  return ((char *)_AX);

EOS:
  return ((char*)_AX);
}

/* ----------------------------------------------------------------------- *

  Name		strdigits

  Declaration	int strdigits(const char * str)

  Description	Counts the number of digits in a string, skipping the
		trailing zeros and spaces (space, tab, carriage return,
		new line, vertical tab, or formfeed).

  Return value	The number of digits in the string or, if a non-digit
		character is found returns its position in the string
		with the minus sign. If str is NULL, -1 is returned.

 * ----------------------------------------------------------------------- */

int strdigits(const char * str)
{
    asm     mov     bx, str
    asm     or      bx, bx		// test if str is NULL
    asm     je      Error1
    asm     mov     dx, bx

Space:
    asm     mov     al, [bx]
    asm     inc     bx
    if (((unsigned char)_AL >= 0x09 && (unsigned char)_AL <= 0x0D) ||
        ((unsigned char)_AL == 0x20)) goto Space;

    asm     dec     bx

Zero:
    asm     mov     al, [bx]
    asm     inc     bx
    asm     cmp     al, '0'
    asm     je      Zero

    asm     mov     ax, bx
    asm     sub     ax, dx
    asm     mov     dx, ax

    asm     dec     bx
    asm     mov     cx, bx

NextChar:
    asm     mov     al, [bx]
    asm     or      al, al
    asm     je      Ok

    asm     cmp     al, '0'
    asm     jb      Error
    asm     cmp     al, '9'
    asm     ja      Error
    asm     inc     bx
    asm     jmp     short NextChar

Ok:
    asm     mov     ax, bx
    asm     sub     ax, cx
    return (_AX);

Error:
    asm     mov     ax, bx
    asm     sub     ax, cx
    asm     add     ax, dx
    asm     neg     ax
    return (_AX);

Error1:
      return (-1);
}

/* ----------------------------------------------------------------------- *

  Name		str_is_unumber

  Declaration	int str_is_unumber(const char * str, int maxdigits)

  Description	Checks if a string contains a valid unsigned integer number.
		Maxdigits is the largest number of digits of the unsigned
		integer number.

  Return value  Non-zero on success, zero otherwise.

 * ----------------------------------------------------------------------- */


int str_is_unumber(const char * str, int maxdigits)
{
    int res = strdigits(str);
    return ((res >= 0) && (res <= maxdigits));
}

void removetrailingspaces(char* str)
{
    char* p = strchr(str, 0); p--;

    while (((*p >= 0x09 && *p <= 0x0D) || (*p == 0x20)) && p > str) p--;
    *(p + 1) = '\0';
}

char* removeleadingspaces(char* str)
{
    while (((*str >= 0x09 && *str <= 0x0D) || (*str == 0x20)) && *str) str++;
    return (str);
}

long ipcheck(char* str, unsigned int & result)
{
    if (*str == '\0') return (-1L);
    removetrailingspaces(str);
    str = removeleadingspaces(str);

    if (!str_is_uint(str)) return (-1L);
    long x = atol(str);
    if (x < 0 || x > 0xFFFF) return (-1L);
    if ((!x) && (strxchr(str, '0'))) return (-1L);
    result = (unsigned)x;
    return (x);
}

int get2uint(char* str, unsigned int& i, unsigned int& j)
{
    char* comma = strchr(str, ',');
    if (!comma) return (0);
    comma++;
    if (ipcheck(comma, j) < 0L) return (0);
    char temp = *(comma - 1);
    *(comma - 1) = '\0';
    if (ipcheck(str, i) < 0L) return (0);
    *(comma - 1) = temp;
    return (1);
}
