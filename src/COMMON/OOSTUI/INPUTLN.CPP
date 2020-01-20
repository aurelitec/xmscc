#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <alloc.h>

#include "common\oostui\oostui.h"
#include <conio.h>
#include "common\tkeys.h"
#include "common\macros.h"
#include "common\tools.cpp"

#define		OOSTUI_TSInputLine	0x0004

class TSInputLine : public TSView
{

public:
  /* TSInputLine palette */
  /* 0 - background      */
  /* 1 - markers         */
    char palette[2];

    TSInputLine(int aLeft, int aTop, int aRight, int aBottom, int aMaxLen);
    virtual ~TSInputLine(void);

    virtual unsigned int isA(void)
      {  return (OOSTUI_TSInputLine);  }

    virtual void draw();
    virtual int valid(int c);
    virtual void handleKeyEvent(unsigned& key);

    char* data;
    int maxLen;
    int curPos;
    int firstPos;
    char cursorIns;

protected:

    int drawFlag;

};

const int CONTROL_Y = 25;

TSInputLine::TSInputLine(int aLeft, int aTop, int aRight, int aBottom,
			 int aMaxLen) :
    TSView( aLeft, aTop, aRight, aBottom ),
    data( (char*)malloc(aMaxLen) ),
    maxLen( aMaxLen ),
    curPos( 0 ),
    firstPos( 0 ),
    cursorIns(0),
    drawFlag(1)
{
  if (!data) sviewhalt();
  *data = EOS;
}

TSInputLine::~TSInputLine()
{
  free(data);
}

void TSInputLine::draw()
{
  if (drawFlag)
  {
    window(left - 1, top, right + 2, bottom);
    textattr(*(palette + 1));
    putch('[');

    static char buf[256];

    memset(buf, '.', width);
    *((char*)memccpy(buf, data+firstPos, 0, width) - 1) = '.';
    buf[width] = EOS;
    textattr(*palette);
    cputs(buf);

    textattr(*(palette + 1));
    putch(']');

    gotoxy(curPos - firstPos + 2, 1);

    drawFlag = 0;
  }
}

#pragma argsused
int TSInputLine::valid(int c)
{
  return (1);
}

void TSInputLine::handleKeyEvent(unsigned& key)
{
  int i;

  switch (ctrlToArrow(key))
  {
    case kbLeft:
      if (curPos > 0)
      {
	curPos--;
	drawFlag = 1;
      }
      break;
    case kbRight:
      if (curPos < strlen(data))
      {
	curPos++;
	drawFlag = 1;
      }
      break;
    case kbHome:
      if (curPos)
      {
	curPos   = 0;
	drawFlag = 1;
      }
      break;
    case kbEnd:
      size_t temp = strlen(data);
      if (curPos != temp)
      {
	curPos   = temp;
	drawFlag = 1;
      }
      break;
    case kbBack:
      if (curPos > 0)
      {
	strcpy(data+curPos-1, data+curPos);
	curPos--;
	if (firstPos > 0) firstPos--;
	drawFlag = 1;
      }
      break;
    case kbDel:
      if (curPos < strlen(data))
      {
	strcpy(data + curPos, data + curPos + 1);
	drawFlag = 1;
      }
      break;
    case kbIns:
      cursorIns = (!cursorIns);
      _setcursortype(cursorIns ? _SOLIDCURSOR : _NORMALCURSOR);
      return;
    default:
      if ((LOBYTE(key) >= ' ') && valid(LOBYTE(key)))
      {
	if ((cursorIns != 0) && (curPos < strlen(data)))
	  strcpy(data + curPos, data + curPos + 1);
	if (strlen(data) < maxLen)
	{
	  if (firstPos > curPos) firstPos = curPos;
	  memmove(data + curPos + 1, data + curPos, strlen(data+curPos)+1);
	  data[curPos++] = LOBYTE(key);
	  drawFlag = 1;
	}
      }
      else
	if (LOBYTE(key) == CONTROL_Y)
	{
	  *data = EOS;
	  curPos = 0;
	  drawFlag = 1;
	}
	else
	  return;
  }
  if (firstPos > curPos) firstPos = curPos;
  i = curPos - width;
  if (firstPos < i) firstPos = i;
  draw();
  key = 0;
}

#define		OOSTUI_TSInputInteger	0x0100

class TSInputInteger : public TSInputLine
{

public:

    TSInputInteger(int aLeft, int aTop, int aRight, int aBottom, int aMaxLen,
		   int aSign = 0) :
	TSInputLine(aLeft, aTop, aRight, aBottom, aMaxLen),
	sign(aSign)
    {
    }

    virtual unsigned int isA(void)
      {  return (OOSTUI_TSInputInteger);  }

    virtual int valid(int c);

protected:

  int sign;

};

int TSInputInteger::valid(int c)
{
  if (sign && (c == '-') && !curPos) return (1);
  return (isdigit(c));
}
