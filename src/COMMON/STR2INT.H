/*------------------------------------------------------------------------*/
/*                                                                        */
/*  String to integer conversion routines                                 */
/*                                                                        */
/*  STR2INT.H: C++ Header File                                            */
/*                                                                        */
/*  Version 1.0 (Jul 16 1997)                                             */
/*  Copyright (c) 1997 Aurelitec (https://www.aurelitec.com)                    */
/*                                                                        */
/*  Written and compiled in Borland C++ 3.1.                              */
/*                                                                        */
/*  Licensed under the MIT License. See LICENSE file in the project       */
/*  root for full license information.                                    */
/*                                                                        */
/*------------------------------------------------------------------------*/

#if !defined(_STR2INT_H)
#define _STR2INT_H

char *strxchr(const char *s, int c);
int strdigits(const char *str);
int str_is_unumber(const char *str, int maxdigits);
void removetrailingspaces(char *str);
char *removeleadingspaces(char *str);
long ipcheck(char *str, unsigned int &result);
int get2uint(char *str, unsigned int &i, unsigned int &j);

#define str_is_uint(str) str_is_unumber(str, 5)
#define str_is_ulong(str) str_is_unumber(str, 10)

#endif // _STR2INT_H