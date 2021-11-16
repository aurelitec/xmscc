/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TReadOnlyINIFile                                                      */
/*  a DOS C++ class to read configuration files (.INI)                    */
/*                                                                        */
/*  READINI.CPP: C++ Source Code                                          */
/*                                                                        */
/*  Version 1.0a (Jul 21 1997)                                            */
/*  Copyright (c) 1997 Aurelitec (https://www.aurelitec.com)                    */
/*                                                                        */
/*  Written and compiled in Borland C++ 3.1.                              */
/*                                                                        */
/*  Licensed under the MIT License. See LICENSE file in the project       */
/*  root for full license information.                                    */
/*                                                                        */
/*------------------------------------------------------------------------*/
/*  HISTORY                                                               */
/*------------------------------------------------------------------------*/
/*                                                                        */
/*  Version 1.0a (Jul 21 1997)                                            */
/*                                                                        */
/*    -> openINI has a new parameter (exepath) to allow search for the    */
/*       .INI file in the directory where the program .EXE file resides   */
/*    -> improved openINI error handling (defined INIERR_xxxx constants)  */
/*    -> changed member function name (shutdown -> closeINI)              */
/*                                                                        */
/*------------------------------------------------------------------------*/

#include <io.h>
#include <fcntl.h>
#include <alloc.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dir.h>

#include "common\readini.h"
#include "common\str2int.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

int TReadOnlyINIFile::openINI(const char *inifilename, const char *exepath,
                              char commentdelimitator)
{
    int handle;
    if ((handle = open(inifilename, O_RDONLY | O_TEXT)) == -1)
        if (exepath)
        {
            char drive[MAXDRIVE];
            char dir[MAXDIR];
            char file[MAXFILE];
            char file2[MAXFILE];
            char ext[MAXEXT];
            char ext2[MAXEXT];
            char path[MAXPATH];

            fnsplit(inifilename, drive, dir, file2, ext2);
            fnsplit(exepath, drive, dir, file, ext);
            fnmerge(path, drive, dir, file2, ext2);

            if ((handle = open(path, O_RDONLY | O_TEXT)) == -1)
                return (INIERR_NOFILE);
        }
        else
            return (INIERR_NOFILE);

    int retvalue = 0;

    closeINI();
    commentchar = commentdelimitator;

    long len = filelength(handle);
    if (len > 65534L)
        retvalue = INIERR_64K;
    else
    {
        datasize = (unsigned int)len;
        data = (char *)malloc(datasize + 1);
        if (data)
            data[read(handle, data, datasize)] = '\0';
        else
            retvalue = INIERR_NOMEM;
    }

    close(handle);
    return (retvalue);
}

int TReadOnlyINIFile::verifyline(char *p)
{
    if (!p)
        return (0);
    p--;
    while (p > data)
    {
        if (*p == '\n')
            return (1);
        if (!isspace(*p))
            return (0);
        p--;
    }
    return (1);
}

int TReadOnlyINIFile::findheader(const char *header)
{
    if (!data)
        return (0);

    char buf[MAXLINELEN];

    strcpy(buf, "[");    /* the opening bracket [	*/
    strcat(buf, header); /* the header name		*/
    strcat(buf, "]");    /* the closing bracket ]	*/

    /* find header */
    char *p = strstr(data, buf);

    if (verifyline(p))
    {
        offset = p;
        return (1);
    }

    return (0);
}

int TReadOnlyINIFile::readstring(const char *key, const char *defaultvalue,
                                 char *value, int maxlen)
{
    if (!data)
        return (0);

    char buf[MAXLINELEN];

    strcpy(buf, key); /* the key name			*/
    strcat(buf, "="); /* the equal sign		*/

    char *p = strstr(offset, buf);

    if (verifyline(p))
    {
        p += strlen(buf);
        char *q = p;
        while (*q != commentchar && *q != '\n' && *q != '\r' && *q != '\0')
            q++;
        int len = min(maxlen, q - p);
        strncpy(value, p, len);
        value[len] = '\0';
        return (1);
    }
    strcpy(value, defaultvalue);
    return (0);
}

/*
 * TReadOnlyINIFile::readuint returns:
 * -1 if the key is not found
 * 0 if the value is an invalid number
 * 1 otherwise
 */

int TReadOnlyINIFile::readuint(const char *key, unsigned int defaultvalue,
                               unsigned int &value)
{
    if (!data)
        return (-1);

    char buf[MAXLINELEN];
    int retval = 0;
    if (readstring(key, "", buf))
    {
        if (ipcheck(buf, value) >= 0L)
            return (1);
    }
    else
        retval = -1;
    value = defaultvalue;
    return (retval);
}

void TReadOnlyINIFile::closeINI(void)
{
    if (data)
        free(data);
    data = 0;
    datasize = 0;
    offset = 0;
}