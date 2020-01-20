/*------------------------------------------------------------------------*/
/*                                                                        */
/*  XMS Control Center Version 1.5 final                                  */
/*  XMS management & information utility                                  */
/*                                                                        */
/*  RAMVIEW.CPP: Program source code                                      */
/*                                                                        */
/*  Created in May 1997                                                   */
/*  Copyright (c) 1997 Devliber (https://devliber.com)                    */
/*                                                                        */
/*  Written and compiled in Borland C++ 3.1.                              */
/*                                                                        */
/*  Licensed under the MIT License. See LICENSE file in the project       */
/*  root for full license information.                                    */
/*                                                                        */
/*------------------------------------------------------------------------*/
/*                                                                        */
/*  Important release                                                     */
/*  revised for publishing with PC-WELT Germany and for uploading         */
/*  to Slovak Antivirus Center (SAC)                                      */
/*                                                                        */
/*  July 18 1997  12:00pm	1.5 beta A                                    */
/*  July 20 1997   8:00pm	1.5 beta B                                    */
/*  July 21 1997   4:19pm	1.5 final beta                                */
/*  July 29 1997   7:18pm	1.5 final                                     */
/*                                                                        */
/*------------------------------------------------------------------------*/
/*                                                                        */
/*  What's new in version 1.5
/*                                                                        */
/*  ++ Configuration file (.INI) to allow user to change colors,          */
/*     confirmations and other options                                    */
/*  +  Added new switch: /NOCFG - use the default configuration (ignore   */
/*     the .INI file)                                                     */
/*  +  Added new switch: /I - ignore configuration errors                 */
/*  +  Added license and contact information in the help screen           */
/*  +  Improved documentation                                             */
/*  +  Fixed a bug                                                        */
/*  +  Inform the user if the XMS driver is HIMEM.SYS                     */
/*                                                                        */
/*------------------------------------------------------------------------*/

#include "xms\xms.h"

#include "common\fancytd.h"
#include "common\oostui\oostui.cpp"
#include "common\oostui\inputln.cpp"
#include "common\oostui\buttons.cpp"
#include "xms\xmstable.cpp"
#include "common\str2int.h"
#include "common\str2int.cpp"
#include "common\readini.h"
#include "common\readini.cpp"

#include "common\types.h"
#include "common\macros.h"

#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <alloc.h>
#include <dir.h>

#define NEWLINE "\r\n"
#define ERRCODE_NOXMS 1
#define ERRCODE_VDISK 2
#define ERRCODE_NOMEM 3

char *errstr_paramformat = "Parameter format not correct - %s\n";
char *errstr_invalidkeyword = "Invalid keyword - %s\n";
char *errstr_noxms = "XMS Driver not Installed";
char *errstr_nomem = "Out of memory";
char *errstr_noerror = "Done.";

char *SM_PrgHeader1 = "XMS Control Center    XMS management and information utility    Version 1.5";
char *SM_PrgHeader2 = "Copyright (c) 1997 Devliber. July 29 1997. Type XMSCC /? for help\n";

char *SM_EMBHeader1 = "EMBs and Available Handles";
char *SM_EMBHeader2 = "EMBs";
char *SM_HandleHeader = "Block  Handle  Size(KB) 32-bit Base Address Lock count        Status";
char *SM_Wait = "Checking Extended Memory Blocks. Please wait...";
char *SM_XMSInfoHeader = "XMS and Extended Memory Usage Information";

int infotype = 1;
int statusLineFlag = 1;
unsigned int maxhandles = 255U;

struct
{
    unsigned int ConfirmFreeEMB : 1;
    unsigned int ConfirmLockEMB : 1;
    unsigned int ConfirmUnlockEMB : 1;
    unsigned int readINI : 1;
    unsigned int commands : 1;
    unsigned int userchoice : 1;
    unsigned int interactive : 1;
    unsigned int isHIMEM : 1;
    unsigned int showINIerrors : 1;
} options;

unsigned int freeHandles;
unsigned long allocatedXM;

void (*readHandles)(void);

XMSHandleTable far *xmstable;

unsigned int far *sHandleTable;
unsigned int sHandleTableTop;

/* Color definitions */

/* Screen       					*/
unsigned char cBackground = MAKEATTR(8, 15);
unsigned char cAppTitleBar = MAKEATTR(9, 15);
unsigned char cAppInfoBar = MAKEATTR(15, 0);
/* XMS handles dialog box				*/
unsigned char cHandleBoxNormal = MAKEATTR(9, 15);
unsigned char cHandleBoxCaptionBar = MAKEATTR(15, 1);
unsigned char cHandleBoxHeader = MAKEATTR(11, 0);
unsigned char cShortCutEnabled = MAKEATTR(11, 12);
unsigned char cShortCutDisabled = MAKEATTR(11, 8);
unsigned char cLabelEnabled = MAKEATTR(11, 0);
unsigned char cLabelDisabled = MAKEATTR(11, 8);
/* XMS handles list box					*/
unsigned char cHandleListNormal = MAKEATTR(9, 15);
unsigned char cHandleListSelected = MAKEATTR(0, 15);
/* XMS information dialog box				*/
unsigned char cInfoBoxNormal = MAKEATTR(9, 15);
unsigned char cInfoBoxCaptionBar = MAKEATTR(15, 1);
/* Red dialog box					*/
unsigned char cRedBoxNormal = MAKEATTR(4, 15);
unsigned char cRedBoxCaptionBar = MAKEATTR(15, 1);
unsigned char cRedBoxFrame = MAKEATTR(4, 15);
unsigned char cRedBoxAlertText = MAKEATTR(4, 14);
/* Command buttons (for red dialog boxes)		*/
unsigned char cButtonInactive = MAKEATTR(7, 0);
unsigned char cButtonActive = MAKEATTR(15, 1);
unsigned char cButtonShadow = MAKEATTR(4, 0);
/* Input dialog box					*/
unsigned char cInputBoxNormal = MAKEATTR(11, 0);
unsigned char cInputBoxCaptionBar = MAKEATTR(15, 1);
unsigned char cInputBoxFrame = MAKEATTR(11, 0);
/* Edit fields (for input dialog boxes)			*/
unsigned char cEditNormal = MAKEATTR(0, 15);
unsigned char cEditMarkers = MAKEATTR(11, 0);
/* Wait dialog box					*/
unsigned char cWaitBoxNormal = MAKEATTR(9, 14);
unsigned char cWaitBoxCaptionBar = MAKEATTR(9, 14);
unsigned char cWaitBoxFrame = MAKEATTR(9, 15);

char tempCharBuf[80];

//--------------------------------------------------------------------------
// display help and exit
//--------------------------------------------------------------------------

void showHelp(void)
{
    puts("To enter interactive mode use the following syntax:");
    puts("  XMSCC [/XMS] [/DV] [/NOCFG] [/I]\n");
    puts("To print an XMS summary to the standard output use the following syntax:");
    puts("  XMSCC /P [/XMS]\n");
    puts("To enter command-line mode use the following syntax:");
    puts("  XMSCC [/F<handle>] [/L<handle>] [/U<handle>] [/R<handle>,<Ksize>] [/A<Ksize>]\n");
    puts("  /XMS                Use XMS calls to get information about EMBs even if");
    puts("                      HIMEM.SYS version 3.09 or above is detected. By default");
    puts("                      XMSCC uses HIMEM.SYS because this method is faster.");
    puts("  /DV                 Use direct video RAM output in interactive mode.");
    puts("  /NOCFG              Disable read configuration (use defaults).");
    puts("  /I                  Ignore configuration errors.");
    puts("  /F<handle>          Free the extended memory block (EMB) addressed by handle.");
    puts("  /L<handle>          Lock the EMB addressed by handle.");
    puts("  /U<handle>          Unlock the EMB addressed by handle.");
    puts("  /R<handle>,<Ksize>  Reallocate the EMB addressed by handle. Ksize is the new");
    puts("                      size of the block in K-bytes.");
    printf("\r\nPress any key to continue...");
    biosGetKey();
    puts("\n\n  /A<Ksize>           Allocate a new EMB of Ksize K-bytes.\n");
    puts("You may freely use, copy and distribute XMS Control Center.");
    puts("Licensed under the MIT License. See LICENSE file for full license information.\n");

    puts("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR");
    puts("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
    puts("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE");
    puts("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
    puts("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
    puts("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE");
    puts("SOFTWARE.\n\n");

    exit(0);
}

//--------------------------------------------------------------------------
// draw interactive mode screen
//--------------------------------------------------------------------------

void drawScreen(void)
{
    textmode(C80);
    biosSetBlink(0);           // disable blinking (use light background colors
    _setcursortype(_NOCURSOR); // hide cursor

    win(1, 1, 80, 25, cBackground); // clear screen

    // display application title in the center of the upper line
    win(1, 1, 80, 1, cAppTitleBar);
    ccputs("XMS Control Center", 1);

    // display version and author information
    win(1, 2, 80, 2, cAppInfoBar);
    cputs(" Version 1.5 ³ Copyright (c) 1997 Devliber");
    gotoxy(68, 1);
    cputs("³ F10 = Exit");
}

void drawStatusLine(void)
{
    int cShortcut1;
    int cLabel1;

    if (infotype)
    {
        cShortcut1 = statusLineFlag ? cShortCutEnabled : cShortCutDisabled;
        cLabel1 = statusLineFlag ? cLabelEnabled : cLabelDisabled;
    }
    else
    {
        cShortcut1 = cShortCutEnabled;
        cLabel1 = cLabelEnabled;
    }

    win(3, 12, 76, 12, cLabelEnabled);
    gotoxy(2, 1);

    textattr(cShortcut1);
    cputs(" F2");
    textattr(cLabel1);
    cputs(" Free ");
    textattr(cShortcut1);
    cputs(" F3");
    textattr(cLabel1);
    cputs(" Lock ");
    textattr(cShortcut1);
    cputs(" F4");
    textattr(cLabel1);
    cputs(" Unlock ");
    textattr(cShortcut1);
    cputs(" F5");
    textattr(cLabel1);
    cputs(" Reallocate ");
    textattr(cShortCutEnabled);
    cputs(" F6");
    textattr(cLabelEnabled);
    cputs(" Allocate new block");
}

void shutdown(const char *custommsg, int errorcode)
{
    _setcursortype(_NORMALCURSOR);
    win(1, 1, 80, 25, MAKEATTR(0, 7));
    if (!custommsg)
        pxmserror("Fatal error");
    else
        printf("%s\n", custommsg);
    exit(errorcode);
}

//--------------------------------------------------------------------------
// TSHandleBox class, derived from TSListBox
//--------------------------------------------------------------------------

class TSHandleBox : public TSListBox
{

public:
    TSHandleBox(int aLeft, int aTop, int aRight, int aBottom, int aMax)
        : TSListBox(aLeft, aTop, aRight, aBottom, aMax)
    {
    }

    virtual void handleKeyEvent(unsigned &key);
    virtual char *getString(int item);
};

void TSHandleBox::handleKeyEvent(unsigned &key)
{
    unsigned char lastFlag;

    if (infotype)
        lastFlag = (xmstable->table + curItem - 1)->flag;
    TSListBox::handleKeyEvent(key);
    if (infotype)
        if (!key)
        {
            unsigned char curFlag = (xmstable->table + curItem - 1)->flag;
            if (curFlag != lastFlag)
            {
                statusLineFlag = (curFlag == 0x02);
                drawStatusLine();
            }
        }
}

char *TSHandleBox::getString(int item)
{
    static char buffer[100];
    memset(buffer, ' ', width);

    if (infotype)
    {
        XMSHandle far *xh = xmstable->table + item - 1;

        if (xh->flag == 0x02 || xh->flag == 0x01)
            sprintf(buffer, "%4u   %5u%10lu            %8lu         %3u         %s",
                    item, FP_OFF(xh), xh->Ksize, xh->Kaddress << 10, xh->lockcount,
                    xh->flag == 0x02 ? "in use" : "free");

        switch (xh->flag)
        {
        case 0x02:
            sprintf(buffer,
                    "%4u   %5u%10lu            %8lu         %3u         in use",
                    sHandleTable[item - 1], FP_OFF(xh), xh->Ksize,
                    xh->Kaddress << 10, xh->lockcount);
            break;
        case 0x01:
            sprintf(buffer,
                    "       %5u%10lu            %8lu         %3u           free",
                    FP_OFF(xh), xh->Ksize, xh->Kaddress << 10, xh->lockcount);
            break;
        default:
            sprintf(buffer, "       %5u%+57s", FP_OFF(xh), "available");
        }
    }
    else
    {
        unsigned int handle = sHandleTable[item - 1];
        unsigned long lockadr;
        unsigned long size = 0;
        unsigned int lockcount = 0;
        freeHandles = 0;

        lockadr = xmlock(handle);
        if (!xmserrno)
            xmunlock(handle);
        xmhandleinfo(handle, &(unsigned int)size, &(unsigned char)lockcount, &(unsigned char)freeHandles);
        sprintf(buffer, "%4u   %5u%10lu            %8lu         %3u         in use",
                item, handle, size, lockadr, lockcount);
    }

    *(buffer + strlen(buffer)) = ' ';
    buffer[width - 1] = EOS;
    return (buffer);
}

TSHandleBox handleBox(4, 6, 76, 11, 0);

unsigned int getHandle(int item)
{
    if (infotype)
        return (FP_OFF(xmstable->table + item - 1));
    else
        return (sHandleTable[item - 1]);
}

void infoTypeWarning(void)
{
    if (options.userchoice)
        strcpy(tempCharBuf, "Not using HIMEM.SYS v3.09+ even if detected");
    else
        strcpy(tempCharBuf, "HIMEM.SYS v3.09 or above not detected");

    TSDialogBox dlg(16, 9, 65, 17, tempCharBuf);
    setPalette(dlg.palette, cRedBoxNormal, cRedBoxCaptionBar, cRedBoxFrame, 0);
    dlg.open();
    gotoxy(2, 2);
    cputs("Slow XMS calls will be used to get information  about the currently allocated eXtended Memory   Blocks (EMBs).");

    TSButtonList b(23, 11, 58, 15);
    setPalette(b.palette, cButtonInactive, cButtonActive, cButtonShadow, 0);

    TSButtonData cmdOK = {13, 4, 21, 4, "OK"};
    b.add(&cmdOK);
    b.draw();

    executeKey(&b, kbEnter, kbEsc);
    dlg.close();
}

void XMSErrorBox(const char *title, const char *errormsg)
{
    TSDialogBox dlg(13, 10, 68, 16, title);
    setPalette(dlg.palette, cRedBoxNormal, cRedBoxCaptionBar, cRedBoxFrame, 0);
    dlg.open();
    ccputs(errormsg, 2);

    TSButtonList b(23, 11, 58, 15);
    setPalette(b.palette, cButtonInactive, cButtonActive, cButtonShadow, 0);
    TSButtonData cmdOK = {13, 3, 21, 3, "OK"};
    b.add(&cmdOK);
    b.draw();

    executeKey(&b, kbEnter, kbEsc);
    dlg.close();
}

void handles2many(void)
{
    long mh = farcoreleft() / 2 - 1;
    if (mh <= maxhandles)
        maxhandles++;
    else
    {
        if (mh > 0xFFFFL)
            mh = 0xFFFFL;
        maxhandles = (unsigned int)mh;
    }

    sHandleTable = (unsigned int far *)farrealloc(sHandleTable, ((long)maxhandles + 1) * 2);
    if (!sHandleTable)
        shutdown(errstr_nomem, ERRCODE_NOMEM);
}

//--------------------------------------------------------------------------
// Retrieve XMS handle information functions:
//	void HIMEM_ReadHandles(void);		- using HIMEM.SYS v3.09+
//	void STANDARD_ReadHandles(void);	- using XMS
//--------------------------------------------------------------------------

void HIMEM_ReadHandles(void)
{
    allocatedXM = 0;
    freeHandles = 0;
    unsigned int blockCount = 0;
    sHandleTableTop = 0xFFFF;
    for (unsigned int i = 0; i < xmstable->handleno; i++)
    {
        sHandleTableTop++;
        if (sHandleTableTop > maxhandles)
            handles2many();
        if ((xmstable->table + i)->flag == 0x02)
        {
            allocatedXM += (xmstable->table + i)->Ksize;
            blockCount++;
            sHandleTable[sHandleTableTop] = blockCount;
        }
        else
        {
            freeHandles++;
            sHandleTable[sHandleTableTop] = 0;
        }
    }
}

void STANDARD_ReadHandles(void)
{
    unsigned hxmem;
    sHandleTableTop = 0xFFFF;
    allocatedXM = 0;

    asm     xor     dx, dx
    asm     mov     hxmem, dx

Again:
    asm     mov     ah, 0x0E
    asm     call    dword ptr [xmscontrol]
    asm     or      ax, ax
    asm     je      Loop
    LOBYTE(freeHandles) = _BL;

    allocatedXM += _DX;
    sHandleTableTop++;
    if (sHandleTableTop > maxhandles) handles2many();
    sHandleTable[sHandleTableTop] = hxmem;

Loop:
    asm     mov     ax, hxmem
    asm     inc     ax
    asm     mov     dx, ax
    asm     mov     hxmem, ax
    asm     cmp     ax, 0xFFFF
    asm     jb      Again

    asm     mov     byte ptr [freeHandles + 1], 0
}


//--------------------------------------------------------------------------
// Update XMS and handle information
//--------------------------------------------------------------------------

char XMSInfo[11][40];

void readXMSInfo(void)
{

    /* XMS version			*/
    unsigned ver = xmsver();
    sprintf(XMSInfo[0], "          XMS Version: %X.%X", HIBYTE(ver), LOBYTE(ver));

    /* XMS driver version		*/
    int hma;
    xmsverinfo(&ver, &hma);
    if (options.isHIMEM)
        sprintf(XMSInfo[1], "       Driver Version: %X.%X (HIMEM.SYS)", HIBYTE(ver), LOBYTE(ver));
    else
        sprintf(XMSInfo[1], "       Driver Version: %X.%X", HIBYTE(ver), LOBYTE(ver));

    /* A20 state			*/
    int a20 = queryA20();
    sprintf(XMSInfo[2], "     A20 Address Line: %s",
            xmserrno ? "???" : (a20 ? "enabled" : "disabled"));
    if (xmserrno == XE_VDISK)
        shutdown(0, ERRCODE_VDISK);

    /* HMA state			*/
    int gothma = hmarequest(0xFFFF);
    if (gothma)
        strcpy(tempCharBuf, "available");
    else
        switch (xmserrno)
        {
        case XE_NO_HMA:
            strcpy(tempCharBuf, "does not exist");
            break;
        case XE_NO_FREEHMA:
            strcpy(tempCharBuf, "in use");
            break;
        case XE_NOTIMPLEMENTED:
            strcpy(tempCharBuf, "no support");
            break;
        case XE_VDISK:
            shutdown(0, ERRCODE_VDISK); // one-way trip
        default:
            strcpy(tempCharBuf, "???");
        }
    sprintf(XMSInfo[3], "     High Memory Area: %s", tempCharBuf);
    if (gothma)
        hmarelease();

    /* Number of available handles	*/
    sprintf(XMSInfo[4], "    Available Handles: %d", freeHandles);

    /* Largest available UMB	*/
    unsigned int us = 0xFFFF;
    unsigned int u = umballoc(&us);
    switch (xmserrno)
    {
    case XE_NOTIMPLEMENTED:
        strcpy(tempCharBuf, "???");
        break;
    case XE_NO_UMBS:
        strcpy(tempCharBuf, "no UMB available");
        break;
    default:
        sprintf(tempCharBuf, "%uK", us * 16 / 1024);
    }
    sprintf(XMSInfo[5], "Largest available UMB: %s", tempCharBuf);
    if (u)
        umbfree(u);

    /* XMS memory statistics	*/
    unsigned int xmfr = xmfreespace();
    if (xmserrno)
        if (xmserrno == XE_VDISK)
            shutdown(0, ERRCODE_VDISK);
        else
            xmfr = 0;
    unsigned int xmc = xmcontig();
    if (xmserrno)
        if (xmserrno == XE_VDISK)
            shutdown(0, ERRCODE_VDISK);
        else
            xmc = 0;

    sprintf(XMSInfo[6], "              Total: %uK", allocatedXM + xmfr);
    sprintf(XMSInfo[7], "          Allocated: %uK", allocatedXM);
    sprintf(XMSInfo[8], "          Available: %uK", xmfr);
    sprintf(XMSInfo[9], " Largest Free Block: %uK", xmc);

    /* XMS control function address	*/
    sprintf(XMSInfo[10], "XMS Control Address: %+04X:%+04X", HIWORD(xmscontrol), LOWORD(xmscontrol));
}

void XMSInformation(void)
{
    /* Display a wait dialog box while reading XMS handle information	*/
    TSDialogBox dlgWait(15, 11, 65, 15, "", 2);
    setPalette(dlgWait.palette, cWaitBoxNormal, cWaitBoxCaptionBar, cWaitBoxFrame, 0);
    dlgWait.open();
    gotoxy(2, 2);
    cputs(SM_Wait);
    readHandles(); // read handles
    readXMSInfo();
    dlgWait.close();

    /* Display XMS handles dialog box					*/
    TSDialogBox dlgHandles(3, 4, 76, 12, infotype ? SM_EMBHeader1 : SM_EMBHeader2, -1);
    setPalette(dlgHandles.palette, cHandleBoxNormal, cHandleBoxCaptionBar, 0);
    dlgHandles.open();
    win(3, 5, 76, 5, cHandleBoxHeader);
    gotoxy(2, 1);
    cputs(SM_HandleHeader);

    /* Display XMS information dialog box					*/
    TSDialogBox dlgInfo(3, 15, 76, 23, SM_XMSInfoHeader, -1);
    setPalette(dlgInfo.palette, cInfoBoxNormal, cInfoBoxCaptionBar, 0);
    dlgInfo.open();

    /* Display XMS information						*/
    int i;
    window(5, 17, 74, 23);
    for (i = 0; i <= 5; i++)
    {
        cprintf(XMSInfo[i]);
        cputs(NEWLINE);
    }
    window(46, 17, 75, 23);
    for (i = 6; i <= 9; i++)
    {
        cprintf(XMSInfo[i]);
        cputs(NEWLINE);
    }
    cputs(NEWLINE);
    cprintf(XMSInfo[10]);

    int oldrange = handleBox.range;
    handleBox.range = infotype ? xmstable->handleno : sHandleTableTop + 1;
    if (oldrange != handleBox.range)
    {
        handleBox.curItem = 1;
        handleBox.delta = 1;
    }
    handleBox.lastDelta = 0;
    handleBox.draw();

    if (!handleBox.range)
        statusLineFlag = 0;
    else if (infotype)
        statusLineFlag = ((xmstable->table + handleBox.curItem - 1)->flag == 0x02);
    drawStatusLine();
}

//--------------------------------------------------------------------------
// Menu function: free block (F2)
//--------------------------------------------------------------------------

void menuFree(unsigned int handle)
{
    int code = 0;
    if (options.ConfirmFreeEMB)
    {
        sprintf(tempCharBuf, "Free block %u", handleBox.curItem);
        TSDialogBox dlg(17, 7, 64, 19, tempCharBuf);
        setPalette(dlg.palette, cRedBoxNormal, cRedBoxCaptionBar, cRedBoxFrame, 0);
        dlg.open();

        ccputs("Are you sure you want to free the block?", 3);
        textattr(cRedBoxAlertText);
        gotoxy(2, 5);
        cputs("This is generally not a good idea as freeing\r\n the wrong block may lead the system into an\r\n unstable state and may even damage files on\r\n your disk!");

        TSButtonList b(23, 11, 58, 15);
        setPalette(b.palette, cButtonInactive, cButtonActive, cButtonShadow, 0);
        TSButtonData cmdOK = {7, 6, 15, 6, "OK"};
        b.add(&cmdOK);
        TSButtonData cmdCancel = {18, 6, 28, 6, "Cancel"};
        b.add(&cmdCancel);
        b.draw();

        int res = executeKey(&b, kbEnter, kbEsc);
        dlg.close();

        if (res && (b.getFocused() == &cmdOK))
            code = 1;
    }
    else
        code = 1;

    if (code)
        if (!xmfree(handle))
            XMSErrorBox("Can't free block", xmserrormsg(xmserrno));
        else
            XMSInformation();
}

//--------------------------------------------------------------------------
// Menu function: lock handle (F3)
//--------------------------------------------------------------------------

void menuLock(unsigned int handle)
{
    int code = 0;
    if (options.ConfirmLockEMB)
    {
        sprintf(tempCharBuf, "Lock block %u", handleBox.curItem);
        TSDialogBox dlg(17, 10, 63, 16, tempCharBuf);
        setPalette(dlg.palette, cRedBoxNormal, cRedBoxCaptionBar, cRedBoxFrame, 0);
        dlg.open();

        ccputs("Are you sure you want to lock the block?", 2);

        TSButtonList b(23, 11, 58, 15);
        setPalette(b.palette, cButtonInactive, cButtonActive, cButtonShadow, 0);
        TSButtonData cmdCancel = {18, 3, 28, 3, "Cancel"};
        b.add(&cmdCancel);
        TSButtonData cmdOK = {7, 3, 15, 3, "OK"};
        b.add(&cmdOK);
        b.draw();

        int res = executeKey(&b, kbEnter, kbEsc);
        dlg.close();

        if (res && (b.getFocused() == &cmdOK))
            code = 1;
    }
    else
        code = 1;

    if (code)
    {
        xmlock(handle);
        if (xmserrno)
            XMSErrorBox("Locking Failed", xmserrormsg(xmserrno));
        else
            XMSInformation();
    }
}

//--------------------------------------------------------------------------
// Menu function:  unlock handle (F4)
//--------------------------------------------------------------------------

void menuUnlock(unsigned int handle)
{
    int code = 0;
    if (options.ConfirmUnlockEMB)
    {
        sprintf(tempCharBuf, "Unlock block %u", handleBox.curItem);
        TSDialogBox dlg(17, 10, 63, 16, tempCharBuf);
        setPalette(dlg.palette, cRedBoxNormal, cRedBoxCaptionBar, cRedBoxFrame, 0);
        dlg.open();

        ccputs("Are you sure you want to unlock the block?", 2);

        TSButtonList b(23, 11, 58, 15);
        setPalette(b.palette, cButtonInactive, cButtonActive, cButtonShadow, 0);
        TSButtonData cmdCancel = {18, 3, 28, 3, "Cancel"};
        b.add(&cmdCancel);
        TSButtonData cmdOK = {7, 3, 15, 3, "OK"};
        b.add(&cmdOK);
        b.draw();

        int res = executeKey(&b, kbEnter, kbEsc);
        dlg.close();
        if (res && (b.getFocused() == &cmdOK))
            code = 1;
    }
    else
        code = 1;

    if (code)
        if (!xmunlock(handle))
            XMSErrorBox("Unlocking Failed", xmserrormsg(xmserrno));
        else
            XMSInformation();
}

//--------------------------------------------------------------------------
// Menu function:  reallocate block (F5)
//--------------------------------------------------------------------------

void menuReAlloc(unsigned int handle)
{
    sprintf(tempCharBuf, "Reallocate block %u", handleBox.curItem);
    TSDialogBox dlg(23, 11, 58, 15, tempCharBuf);
    setPalette(dlg.palette, cInputBoxNormal, cInputBoxCaptionBar, cInputBoxFrame, 0);
    dlg.open(_NORMALCURSOR);

    gotoxy(3, 2);
    cputs("New size (in K-bytes):");

    TSInputInteger i(27, 2, 31, 2, 5);
    dlg.makeLocal(&i);
    setPalette(i.palette, cEditNormal, cEditMarkers, 0);
    i.draw();

    int res = executeKey(&i, kbEnter, kbEsc);
    dlg.close();

    if (res && i.curPos)
    {
        unsigned size;
        if (ipcheck(i.data, size) < 0L)
            XMSErrorBox("Error", "Value must be between 0 and 65535");
        else
        {
            if (!xmrealloc(handle, size))
                XMSErrorBox("Reallocation Failed", xmserrormsg(xmserrno));
            else
                XMSInformation();
        }
    }
}

//--------------------------------------------------------------------------
//  Menu function: allocate new block (F6)
//--------------------------------------------------------------------------

void menuAlloc(void)
{
    TSDialogBox dlg(24, 11, 56, 15, "Allocate new block");
    setPalette(dlg.palette, cInputBoxNormal, cInputBoxCaptionBar, cInputBoxFrame, 0);
    dlg.open(_NORMALCURSOR);

    gotoxy(3, 2);
    cputs("Size (in K-bytes):");

    TSInputInteger i(47, 13, 51, 13, 5);
    setPalette(i.palette, cEditNormal, cEditMarkers, 0);
    i.draw();

    int res = executeKey(&i, kbEnter, kbEsc);
    dlg.close();

    if (res && i.curPos)
    {
        unsigned size;
        if (ipcheck(i.data, size) < 0L)
            XMSErrorBox("Error", "Value must be between 0 and 65535");
        else
        {
            if (!xmalloc(size))
                XMSErrorBox("Allocation Failed", xmserrormsg(xmserrno));
            else
                XMSInformation();
        }
    }
}

//--------------------------------------------------------------------------
// testcommandline
//--------------------------------------------------------------------------

void testcommandline(void)
{
    int i;
    unsigned int chandle;
    unsigned int csize;
    unsigned long caddress;

    /* test for /?... or /H... in the command line	*/
    for (i = 1; i < _argc; i++)
    {
        char *curarg = *(_argv + i);
        if (*curarg == '/')
        {
            int c = toupper(*(curarg + 1));
            if (c == '?' || c == 'H')
                showHelp(); // display help screen and halt
        }
    }

    /* test if XMS						*/
    if (!initxms())
    {
        puts(errstr_noxms);
        exit(ERRCODE_NOXMS);
    }

    /* process command line				*/
    for (i = 1; i < _argc; i++)
    {
        char *curarg = *(_argv + i);
        int c = toupper(*(curarg + 1));
        char *pp = curarg + 2;

        if (*curarg != '/')
        {
            printf(errstr_invalidkeyword, curarg);
            options.commands = 1;
            continue;
        }

        int commands2 = 1;
        switch (c)
        {

            // Removed in version 1.1
            /*      case 'M':         // set maximum number of handles
	if (ipcheck(pp, chandle) < 0L) printf(errstr_paramformat, curarg);
	else maxhandles = chandle - 1;
	commands2 = 0;
      break;
*/
        case 'F': // free EMB
            if (ipcheck(pp, chandle) < 0L)
                printf(errstr_paramformat, curarg);
            else if (!xmfree(chandle))
                printf("Can't free EMB (handle %u): %s\n", chandle, xmserrormsg(xmserrno));
            else
                printf("EMB (handle %u) freed.\n", chandle);
            break;

        case 'L': // lock EMB
            if (ipcheck(pp, chandle) < 0L)
                printf(errstr_paramformat, curarg);
            else
            {
                caddress = xmlock(chandle);
                if (xmserrno)
                    printf("Can't lock EMB (handle %u): %s\n", chandle, xmserrormsg(xmserrno));
                else
                    printf("EMB (handle %u) locked. 32-bit linear address is %lu.\n", chandle, caddress);
            }
            break;

        case 'U': // unlock EMB
            if (ipcheck(pp, chandle) < 0L)
                printf(errstr_paramformat, curarg);
            else
            {
                if (!xmunlock(chandle))
                    printf("Can't unlock EMB (handle %u): %s\n", chandle, xmserrormsg(xmserrno));
                else
                    printf("EMB (handle %u) unlocked.\n", chandle);
            }
            break;

        case 'R': // reallocate EMB
            int error = 0;
            char *comma = strchr(pp, ',');
            if (!comma)
                error = 1;
            else
            {
                comma++;
                if (ipcheck(comma, csize) < 0L)
                    error = 1;
                else
                {
                    char temp = *(comma - 1);
                    *(comma - 1) = '\0';
                    if (ipcheck(pp, chandle) < 0L)
                        error = 1;
                    *(comma - 1) = temp;
                    if (!error)
                    {
                        if (!xmrealloc(chandle, csize))
                            printf("Can't reallocate EMB (handle %u): %s\n", chandle, xmserrormsg(xmserrno));
                        else
                            printf("EMB (handle %u) reallocated. New size: %u K-bytes.\n", chandle, csize);
                    }
                }
            }

            if (error)
                printf(errstr_paramformat, curarg);
            break;

        case 'A': // allocate new EMB
            if (ipcheck(pp, csize) < 0L)
                printf(errstr_paramformat, curarg);
            else if (!(chandle = xmalloc(csize)))
                printf("Can't allocate %u K-bytes: %s\n", csize, xmserrormsg(xmserrno));
            else
                printf("Allocated %u K-bytes. Handle to EMB is %u.\n", csize, chandle);
            break;

        default:
            commands2 = 0;
            if (!strcmp(strupr(*(_argv + i)), "/I"))
                options.showINIerrors = 0;
            else if (!strcmp(strupr(*(_argv + i)), "/NOCFG"))
                options.readINI = 0;
            else if (!strcmp(strupr(*(_argv + i)), "/XMS"))
                infotype = 0;
            else if (!strcmp(strupr(*(_argv + i)), "/DV"))
                directvideo = 1;
            else
            {
                directvideo = 0; // use Video ROM Calls by default, for compatibility
                if (!strcmp(strupr(*(_argv + i)), "/P"))
                    options.interactive = 0;
                else
                {
                    printf("Invalid switch - %s\n", *(_argv + i));
                    commands2 = 1;
                }
            }
        }
        if (commands2)
            options.commands = 1;
    }
}

//--------------------------------------------------------------------------
// display XMS summary and EMB information at standard output
//--------------------------------------------------------------------------

void putunderlined(const char *s)
{
    int hlen = strlen(s);
    puts(s);
    memset(tempCharBuf, '-', hlen);
    tempCharBuf[hlen] = '\0';
    puts(tempCharBuf);
}

void stdoutsum(void)
{
    unsigned int i;

    printf("%s\n\n", SM_Wait);
    readHandles(); // read EMB information
    readXMSInfo(); // read XMS infomation

    putunderlined(SM_XMSInfoHeader);
    puts("");

    for (i = 0; i <= 5; i++)
        puts(*(XMSInfo + i));
    for (i = 6; i <= 10; i++)
        printf("  %s\n", *(XMSInfo + i));
    puts("\n");

    putunderlined(infotype ? SM_EMBHeader1 : SM_EMBHeader2);
    puts("");

    putunderlined(SM_HandleHeader);

    unsigned int maxh = infotype ? xmstable->handleno : sHandleTableTop + 1;
    for (i = 1; i <= maxh; i++)
        puts(handleBox.getString(i));

    exit(0);
}

//--------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------

void new_sviewhalt(void)
{
    shutdown(errstr_nomem, ERRCODE_NOMEM);
}

int INIerror = 0;
TReadOnlyINIFile ini;

void readboolean(const char *headername, const char *keyname,
                 unsigned int defaultvalue, unsigned int &value)
{
    int res = ini.readuint(keyname, defaultvalue, value);
    switch (res)
    {
    case 1:
        if (value <= 1)
            return;
        break;
    case 0:
        if (options.showINIerrors)
            printf("Error in .INI file: [%s] %s contains an invalid value\r\n",
                   headername, keyname);
        INIerror = 1;
    }
}

void readcolor(const char *colorname, unsigned char &color)
{
    char buf[MAXLINELEN];
    unsigned int i;
    unsigned int j;
    if (ini.readstring(colorname, "", buf))
    {
        if (get2uint(buf, i, j))
            if (i <= 15 && j <= 15)
            {
                color = MAKEATTR(i, j);
                return;
            }
        if (options.showINIerrors)
            printf("Error in .INI file: [Colors] %s contains invalid values\r\n", colorname);
        INIerror = 1;
    }
}

void readconfiguration(void)
{
    int inierr = ini.openINI("XMSCC.INI", _argv[0]);
    if (inierr)
    {
        if (options.showINIerrors)
        {
            if (inierr == INIERR_NOFILE)
                return;
            printf("Configuration read error: ");
            switch (inierr)
            {
            case INIERR_64K:
                puts(".INI file is larger than 64K");
                break;
            case INIERR_NOMEM:
                puts("Not enough memory to process .INI file");
            }
            printf("\r\nPress any key to continue...");
            biosGetKey();
        }
        return;
    }

    if (ini.findheader("General"))
    {
        unsigned int i;
        readboolean("General", "UseDirectVideo", 0, i);
        directvideo = i;
        readboolean("General", "UseXMSCalls", 0, i);
        if (i)
        {
            infotype = 0;
            options.userchoice = 1;
        }
    }

    if (ini.findheader("Confirmations"))
    {
        unsigned int i;
        readboolean("Confirmations", "ConfirmFreeEMB", 1, i);
        options.ConfirmFreeEMB = i;
        readboolean("Confirmations", "ConfirmLockEMB", 1, i);
        options.ConfirmLockEMB = i;
        readboolean("Confirmations", "ConfirmUnlockEMB", 1, i);
        options.ConfirmUnlockEMB = i;
    }

    if (ini.findheader("Colors"))
    {
        readcolor("Background", cBackground);
        readcolor("AppTitleBar", cAppTitleBar);
        readcolor("AppInfoBar", cAppInfoBar);
        readcolor("HandleBoxNormal", cHandleBoxNormal);
        readcolor("HandleBoxCaptionBar", cHandleBoxCaptionBar);
        readcolor("HandleBoxHeader", cHandleBoxHeader);
        readcolor("ShortCutEnabled", cShortCutEnabled);
        readcolor("ShortCutDisabled", cShortCutDisabled);
        readcolor("LabelEnabled", cLabelEnabled);
        readcolor("LabelDisabled", cLabelDisabled);
        readcolor("HandleListNormal", cHandleListNormal);
        readcolor("HandleListSelected", cHandleListSelected);
        readcolor("InfoBoxNormal", cInfoBoxNormal);
        readcolor("InfoBoxCaptionBar", cInfoBoxCaptionBar);
        readcolor("RedBoxNormal", cRedBoxNormal);
        readcolor("RedBoxCaptionBar", cRedBoxCaptionBar);
        readcolor("RedBoxFrame", cRedBoxFrame);
        readcolor("RedBoxAlertText", cRedBoxAlertText);
        readcolor("ButtonInactive", cButtonInactive);
        readcolor("ButtonActive", cButtonActive);
        readcolor("ButtonShadow", cButtonShadow);
        readcolor("InputBoxNormal", cInputBoxNormal);
        readcolor("InputBoxCaptionBar", cInputBoxCaptionBar);
        readcolor("InputBoxFrame", cInputBoxFrame);
        readcolor("EditNormal", cEditNormal);
        readcolor("EditMarkers", cEditMarkers);
        readcolor("WaitBoxNormal", cWaitBoxNormal);
        readcolor("WaitBoxCaptionBar", cWaitBoxCaptionBar);
        readcolor("WaitBoxFrame", cWaitBoxFrame);
    }

    ini.closeINI();
    if (INIerror && options.showINIerrors)
    {
        printf("\r\nThe configuration file (XMSCC.INI) has errors. Press any key to continue...");
        biosGetKey();
    }
}

int main(void)
{
    sviewhalt = new_sviewhalt;

    options.ConfirmFreeEMB = 1;
    options.ConfirmLockEMB = 1;
    options.ConfirmUnlockEMB = 1;
    options.readINI = 1;
    options.commands = 0;
    options.userchoice = 0;
    options.interactive = 1;
    options.isHIMEM = 0;
    options.showINIerrors = 1;

    // display program header
    puts(SM_PrgHeader1);
    puts(SM_PrgHeader2);

    testcommandline(); // test command-line arguments
    if (options.commands)
        return 0; // command-line mode, already executed

    if (options.interactive && options.readINI)
        readconfiguration();

    // decide what method to use to get EMB information: HIMEM.SYS or XMS
    xmstable = getXMSHandleTable();
    options.isHIMEM = (xmstable != 0);
    if (infotype)
        infotype = options.isHIMEM;
    else
        options.userchoice = 1;
    readHandles = infotype ? HIMEM_ReadHandles : STANDARD_ReadHandles;

    sHandleTable = (unsigned int far *)farmalloc(((long)maxhandles + 1) * 2);
    if (!sHandleTable)
        shutdown(errstr_nomem, ERRCODE_NOMEM);

    // display summary at standard output and end program
    if (!options.interactive)
        stdoutsum();

    // enter interactive mode
    drawScreen();
    if (!infotype)
        infoTypeWarning();
    setPalette(handleBox.palette, cHandleListNormal, cHandleListSelected, 0);
    XMSInformation();

    // run interactive
    unsigned c;
    while (1)
    {
        c = biosGetKey();
        if (c == kbF10)
            break;
        switch (c)
        {
        case kbF2:
            if (statusLineFlag)
                menuFree(getHandle(handleBox.curItem));
            break;
        case kbF3:
            if (statusLineFlag)
                menuLock(getHandle(handleBox.curItem));
            break;
        case kbF4:
            if (statusLineFlag)
                menuUnlock(getHandle(handleBox.curItem));
            break;
        case kbF5:
            if (statusLineFlag)
                menuReAlloc(getHandle(handleBox.curItem));
            break;
        case kbF6:
            menuAlloc();
            break;
        default:
            handleBox.handleKeyEvent(c);
        }
    }

    // restore cursor and text mode
    _setcursortype(_NORMALCURSOR);
    win(1, 1, 80, 25, MAKEATTR(0, 7));
    textmode(LASTMODE);

    farfree(sHandleTable);
    return 0;
}
