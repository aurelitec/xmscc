#include "common\fancytd.h"
#include "common\oostui\oostui.h"
#include "common\tkeys.h"
#include "common\tools.cpp"
#include <conio.h>
#include <alloc.h>
#include <dos.h>

//--------------------------------------------------------------------------
// TSButtonList
//--------------------------------------------------------------------------

#define		OOSTUI_TSButtonList	0x0006

struct TSButtonData
{
  int left;
  int top;
  int right;
  int bottom;
  char* name;
  TSButtonData* next;
  TSButtonData* prev;
  int width;
  int height;
};

class TSButtonList : public TSView
{

protected:
  TSButtonData* curButton;
  TSButtonData* lastButton;

public:
  /* TSButtonList palette */
  /* 0 - inactive item    */
  /* 1 - active item      */
  /* 2 - shadow           */
  char palette[3];

  TSButtonList(int aLeft, int aTop, int aRight, int aBottom);

  virtual unsigned int isA(void)
    {  return (OOSTUI_TSButtonList);  }

  void add(TSButtonData* b);

  TSButtonData* getFocused(void)
    {  return (curButton);  }

  virtual void handleKeyEvent(unsigned& key);

  virtual void draw(void);
  virtual void drawButton(TSButtonData* b, int active, int fast);
};

TSButtonList::TSButtonList(int aLeft, int aTop, int aRight, int aBottom)
	    : TSView(aLeft, aTop, aRight, aBottom),
	      curButton(0),
	      lastButton(0)
{
}

void TSButtonList::add(TSButtonData* b)
{
  if (!b) return;
  if (!curButton)
  {
    curButton = b;
    curButton->next = b;
    curButton->prev = b;
  }
  else
  {
    b->next = curButton->next;
    curButton->next->prev = b;
    curButton->next = b;
    b->prev = curButton;
    curButton = b;
  }
  b->left   += left;
  b->top    += top;
  b->right  += left;
  b->bottom += top;
  b->width  = b->right  - b->left + 1;
  b->height = b->bottom - b->top  + 1;
}

void TSButtonList::handleKeyEvent(unsigned& key)
{
  switch (ctrlToArrow(key))
  {
    case kbLeft:
      lastButton = curButton;
      curButton = curButton->next;
    break;
    case kbRight:
      lastButton = curButton;
      curButton = curButton->prev;
      break;
    default:
      return;
  }
  draw();
  key = 0;
}

void TSButtonList::draw(void)
{
  if (!curButton) return;
  if (!lastButton)	// this means we have to redraw the whole button list
  {
    drawButton(curButton, 1, 0);
    TSButtonData* b = curButton->next;
    while (b != curButton)
    {
      drawButton(b, 0, 0);
      b = b->next;
    }
  }
  else
  {
    drawButton(lastButton, 0, 1);
    drawButton(curButton, 1, 1);
  }
}

void TSButtonList::drawButton(TSButtonData* b, int active, int fast)
{
  static char buffer[80];

//  win(b->left, b->top, b->right, b->bottom, *(palette + active));
  window(b->left, b->top, b->right + 1, b->bottom);
  memset(buffer, ' ', b->width - 1);
  *(buffer + b->width - 1) = EOS;
  textattr(*(palette + active));
  cputs(buffer);

  window(b->left, b->top, b->right, b->bottom);
  ccputs(b->name, 1);
  window(b->left, b->top, b->right + 2, b->bottom + 1);

  if (!fast)
  {
    textattr(*(palette + 2));
    gotoxy(2, 2);
    memset(buffer, 'ß', b->width - 1);
    *(buffer + b->width - 1) = EOS;
    cputs(buffer);
    gotoxy(wherex() - 1, wherey() - 1);
    putch('Ü');
  }
}
