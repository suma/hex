#ifndef CONTROL_H_INC
#define CONTROL_H_INC

#include <QMap>

class Document;
class Cursor;
class View;

class Control
{
protected:
  Document *doc_;
  typedef QMap<int, Cursor*> CursorMap;
  CursorMap curs_;

public:
  Control(Document *doc);
  ~Control();

  bool AddCursor(int id, Cursor *cur);
  bool AddView(int id, View *view);

};


#endif
