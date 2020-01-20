/*------------------------------------------------------------------------*/
/*                                                                        */
/*  XMS Interface for Borland C/C++                                       */
/*                                                                        */
/*  XMS.H: C++ Header File                                                */
/*                                                                        */
/*  Version 1.1 (April 1997)                                              */
/*  Copyright (c) 1997 Devliber (https://devliber.com)                    */
/*                                                                        */
/*  Written and compiled in Borland C++ 3.1.                              */
/*                                                                        */
/*  Licensed under the MIT License. See LICENSE file in the project       */
/*  root for full license information.                                    */
/*                                                                        */
/*------------------------------------------------------------------------*/

#if !defined(__XMS_H)
#define __XMS_H

/* 16-bit handle to an extended memory block
 */

typedef unsigned int xmhandle;

/* Structure used by xmemcpy
 */

typedef struct
{
    unsigned long xmc_count;
    xmhandle xmc_srchandle;
    unsigned long xmc_srcoff;
    xmhandle xmc_desthandle;
    unsigned long xmc_destoff;
} xmemcpy_t;

/* XMS error codes
 */

#define XE_NOERROR 0x00 // no error, successful operation

// miscellaneous errors
#define XE_NOTIMPLEMENTED 0x80 // the function is not implemented
#define XE_API XE_NOTIMPLEMENTED
#define XE_VDISK 0x81         // a VDISK device is detected
#define XE_A20 0x82           // an A20 error occurs
#define XE_DRVFAULT 0x8E      // a general driver error occurs
#define XE_UNRECOVERABLE 0x8F // an unrecoverable driver error occurs

// HMA errors
#define XE_NO_HMA 0x90          // the HMA does not exist
#define XE_NO_FREEHMA 0x91      // the HMA is already in use
#define XE_BAD_HMAMINSIZE 0x92  // DX is less than the /HMAMIN= parameter
#define XE_HMANOTALLOCATED 0x93 // the HMA is not allocated
#define XE_A20ENABLED 0x94      // the A20 line is still enabled

// eXtended Memory errors
#define XE_NO_FREEMEM 0xA0      // all extended memory is allocated
#define XE_NO_FREEHANDLES 0xA1  // all available extended memory handles are in use
#define XE_BAD_HANDLE 0xA2      // the handle is invalid
#define XE_BAD_SRC_HANDLE 0xA3  // the SourceHandle is invalid
#define XE_BAD_SRC_OFF 0xA4     // the SourceOffset is invalid
#define XE_BAD_DEST_HANDLE 0xA5 // the DestHandle is invalid
#define XE_BAD_DEST_OFF 0xA6    // the DestOffset is invalid
#define XE_BAD_LEN 0xA7         // the Length is invalid
#define XE_BAD_OVERLAP 0xA8     // the move has an invalid overlap
#define XE_PARITY 0xA9          // a parity error occurs
#define XE_UNLOCKED 0xAA        // the block is not locked
#define XE_LOCKED 0xAB          // the block is locked
#define XE_LOCKCOUNTOF 0xAC     // the block's lock count overflows
#define XE_LOCKFAIL 0xAD        // the lock fails

// UMB errors
#define XE_UMB2BIG 0xB0    // a smaller UMB is available
#define XE_NO_UMBS 0xB1    // no UMBs are available
#define XE_BAD_UMBSEG 0xB2 // the UMB segment number is invalid

/* XMS error code variable
 */

extern unsigned char xmserrno;

/* Indicator of the existance of an XMS driver
 */

extern unsigned char xmsinstalled;

/* array of XMS error message strings
 */

extern char *xms_errlist[];

/* number of XMS error message strings
 */

extern int xms_nerr;

/* Miscellaneous constants
 */

#define HMASEG 0xFFFF
#define HMASTARTOFF 0x0010
#define HMAENDOFF 0xFFFF

/* pointer to XMS control function
 */

extern void(far *xmscontrol)(void);

#ifdef __cplusplus
extern "C"
{
#endif

    /* Initialization Functions
 */

    int _xmsdrivercheck(void);
    void _getxmsfunct(void);
    int initxms(void); // MUST BE CALLED BEFORE ANY OTHER ROUTINE

    //#pragma startup initxms 100

    /* Driver Information Functions
 */

    unsigned int xmsver(void);
    unsigned int xmsverinfo(unsigned int *revision, int *HMA);

    /* High Memory Area (HMA) Management Functions
 */

    int hmarequest(unsigned int reqsize);
    int hmarelease(void);

    /* A20 Management Functions
 */

    int globalenableA20(void);
    int globaldisableA20(void);
    int localenableA20(void);
    int localdisableA20(void);
    int queryA20(void);

    /* eXtended Memory Management Functions
 */

    unsigned int xmfreespace(void);
    unsigned int xmcontig(void);
    xmhandle xmalloc(unsigned int size);
    int xmfree(xmhandle hxmem);
    unsigned long xmlock(xmhandle hxmem);
    int xmunlock(xmhandle hxmem);
    int xmhandleinfo(xmhandle hxmem, unsigned int *size, unsigned char *lockcount, unsigned char *freehandles);
    int xmrealloc(xmhandle hxmem, unsigned int newsize);

    /* eXtended Memory Transfer Functions
 */

    int xmemcpy(xmemcpy_t *x);
    int _xmemcpy(unsigned long n, xmhandle srchandle, unsigned long srcoff,
                 xmhandle desthandle, unsigned long destoff);

#ifdef __cplusplus
    int inline ctoxm(xmhandle desthandle, unsigned long destoff, void far *src, unsigned long n)
    {
        return (_xmemcpy(n, 0, (unsigned long)src, desthandle, destoff));
    }

    int inline xmtoc(void far *dest, xmhandle srchandle, unsigned long srcoff, unsigned long n)
    {
        return (_xmemcpy(n, srchandle, srcoff, 0, (unsigned long)dest));
    }

    int inline xmtoxm(xmhandle desthandle, unsigned long destoff, xmhandle srchandle, unsigned long srcoff, unsigned long n)
    {
        return (_xmemcpy(n, srchandle, srcoff, desthandle, destoff));
    }

    int inline ctoc(void far *dest, void far *src, unsigned long n)
    {
        return (_xmemcpy(n, 0, (unsigned long)src, 0, (unsigned long)dest));
    }
#else

#define ctoxm(desthandle, destoff, src, n) \
    _xmemcpy((n), 0, (unsigned long)(src), (desthandle), (destoff))

#define xmtoc(dest, srchandle, srcoff, n) \
    _xmemcpy((n), (srchandle), (srcoff), 0, (unsigned long)(dest))

#define xmtoxm(desthandle, destoff, srchandle, srcoff, n) \
    _xmemcpy((n), (srchandle), (srcoff), (desthandle), (destoff))

#define ctoc(dest, src, n) \
    _xmemcpy((n), 0, (unsigned long)(src), 0, (unsigned long)(dest))

#endif

    /* Upper Memory Blocks (UMB) Management Functions
 */

    unsigned int umballoc(unsigned int *size);
    int umbfree(unsigned int umbseg);
    int umbrealloc(unsigned int umbseg, unsigned int *size);

    /* XMS error functions
 */

    char *xmserrormsg(int errorcode);
    void pxmserror(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* __XMS_H */
