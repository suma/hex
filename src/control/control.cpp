#include "control.h"

Control::Control(Document *doc)
  : doc_(doc)
{
}

Control::~Control()
{
}

bool Control::AddCursor(int id, Cursor *cur)
{
  if (curs_.find(id) != curs_.constEnd()) {
    return false;
  }
  curs_.insert(id, cur);
  return true;
}

bool Control::AddView(int id, View *view)
{
  CursorMap::const_iterator i = curs_.find(id);
  if (i == curs_.constEnd()) {
    return false;
  }
  //(*id)->AddView(view);
  return true;
}
