/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TReadOnlyINIFile                                                      */
/*  a DOS C++ class to read configuration files (.INI)                    */
/*                                                                        */
/*  READINI.H: C++ Header File                                            */
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

#if !defined(_READINI_H)
#define _READINI_H

const MAXLINELEN = 81;

#define INIERR_NOFILE -1 // .INI file not found
#define INIERR_64K -2    // .INI file is larger than 64K
#define INIERR_NOMEM -3  // Not enough memory to process .INI file

class TReadOnlyINIFile
{

private:
    char *data;
    unsigned int datasize;
    char *offset;
    char commentchar;

public:
    TReadOnlyINIFile(void);
    int openINI(const char *inifilename, const char *exepath = 0, char commentdelimitator = '#');
    unsigned int INIsize(void) { return (datasize); }
    int findheader(const char *header);
    int readstring(const char *key, const char *defaultvalue, char *value, int maxlen = MAXLINELEN);
    int readuint(const char *key, unsigned int defaultvalue, unsigned int &value);
    void closeINI(void);
    ~TReadOnlyINIFile(void) { closeINI(); }

private:
    int verifyline(char *p);
};

inline TReadOnlyINIFile::TReadOnlyINIFile(void)
{
    data = 0;
    datasize = 0;
    offset = 0;
}

#endif // _READINI_H