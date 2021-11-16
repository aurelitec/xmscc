/*------------------------------------------------------------------------*/
/*                                                                        */
/*  Get XMS handle table using INT 0x2F, function 0x4309,                 */
/*  supported by HIMEM.SYS v3.09+                                         */
/*                                                                        */
/*  Copyright (c) 1997 Aurelitec (https://www.aurelitec.com)                    */
/*                                                                        */
/*  Written and compiled in Borland C++ 3.1.                              */
/*                                                                        */
/*  Licensed under the MIT License. See LICENSE file in the project       */
/*  root for full license information.                                    */
/*                                                                        */
/*  This source file is part of the                                       */
/*  XMS Interface for Borland C/C++ Library Version 1.5                   */
/*  developed by Aurelitec                                                 */
/*                                                                        */
/*------------------------------------------------------------------------*/

#include <dos.h>          // for MK_FP
#include "common\types.h" // for BYTE, WORD and DWORD type definitions

/*  XMS handle descriptor structure
 */

struct XMSHandle
{
    BYTE flag;      // flag (01h = free, 02h = used,
                    // 04h = in pool but not associated with any EMB)
    BYTE lockcount; // lock count (00h = unlocked)
    DWORD Kaddress; // address of XMS block in KB
    DWORD Ksize;    // size of XMS block in KB
};

/*  XMS handle table structure
 */

struct XMSHandleTable
{
    BYTE unknown;         // unknown semnification (01h in HIMEM.SYS v3.09)
    BYTE hsize;           // size of one handle descriptor
    WORD handleno;        // number of handles (default = 20h)
    XMSHandle far *table; // pointer to XMS handle array
};

/* ----------------------------------------------------------------------- *

  Name      getXMSHandleTable

  Declaration   XMSHandleTable far * getXMSHandleTable(void);

  Description   Returns the address of the XMS handle table. The function will
        work with HIMEM.SYS v3.09 or above. HIMEM.SYS v3.09 is part
        of MS-DOS 6.0.

  Return value  If the function is supported returns a pointer to the XMS
        handle table, otherwise a NULL pointer is returned.

 * ----------------------------------------------------------------------- */

XMSHandleTable far * getXMSHandleTable(void)
{
    asm     mov     ax, 0x4309
    asm     int     0x2F
    asm     cmp     al, 0x43
    asm     jne     NotSupported
    return( (XMSHandleTable far *)MK_FP(_ES, _BX) );

NotSupported:
    return (0);
}