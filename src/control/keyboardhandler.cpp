
#include "keyboardhandler.h"
#include "document.h"
#include "commands.h"

KeyboardHandler::KeyboardHandler(Document *doc)
  : document_(doc)
{
}

KeyboardHandler::~KeyboardHandler()
{
}

void KeyboardHandler::keyPressEvent(QKeyEvent *)
{
}

void KeyboardHandler::keyPressEvent(QChar)
{
}

void KeyboardHandler::changeData(quint64 pos, quint64 len, const uchar *data, uint data_len)
{
  Q_ASSERT(pos <= document_->length());

  document_->undoStack()->push(new ReplaceCommand(document_, pos, len, data, data_len));
}

void KeyboardHandler::insertData(quint64 pos, uchar character)
{
  Q_ASSERT(pos <= document_->length());

  insertData(pos, &character, 1);
}

void KeyboardHandler::insertData(quint64 pos, const uchar *data, uint len)
{
  Q_ASSERT(pos <= document_->length());

  document_->undoStack()->push(new InsertCommand(document_, pos, data, len));
}

void KeyboardHandler::removeData(quint64 pos, quint64 len)
{
  Q_ASSERT(pos <= document_->length());

  document_->undoStack()->push(new DeleteCommand(document_, pos, len));
}
