#pragma once

#include <QUndoCommand>
#include <vector>
#include "document.h"


class UndoCommand : public QUndoCommand
{
public:
  UndoCommand(QUndoCommand *parent = 0);

  virtual void setEnable(bool t)
  {
    enable_ = t;
  }

  bool enable() const
  {
    return enable_;
  }
private:
  bool enable_;
};


class InsertCommand : public UndoCommand
{
public:
  InsertCommand(Document *doc, quint64 pos, const uchar *data, uint length, QUndoCommand *parent = 0);
  InsertCommand(Document *doc, const InsertCommand *cmd, QUndoCommand *parent = 0);  // for reconstruct UndoStack
  void undo();
  void redo();

  quint64 position() const
  {
    return position_;
  }

  const DocumentFragment &fragment() const
  {
    return fragment_;
  }

private:
  Document *document_;
  quint64 position_;
  DocumentFragment fragment_;
};


class DeleteCommand : public UndoCommand
{
public:
  DeleteCommand(Document *doc, quint64 pos, quint64 len, QUndoCommand *parent = 0);
  DeleteCommand(Document *doc, const DeleteCommand *cmd, QUndoCommand *parent = 0);  // for reconstruct undoStack
  void undo();
  void redo();

  quint64 position() const
  {
    return position_;
  }

  quint64 length() const
  {
    return length_;
  }

  const Document::FragmentList &fragments() const
  {
    return fragments_;
  }

private:
  Document *document_;
  quint64 position_;
  quint64 length_;
  Document::FragmentList fragments_;
};



class ReplaceCommand : public UndoCommand
{
public:
  ReplaceCommand(Document *doc, quint64 pos, quint64 len, const uchar *data, uint length, QUndoCommand *parent = 0);
  ReplaceCommand(Document *doc, const ReplaceCommand *cmd, QUndoCommand *parent = 0);  // for reconstruct UndoStack
  void undo();
  void redo();

  const DeleteCommand *deleteCommand() const
  {
    return &delete_;
  }

  const InsertCommand *insertCommand() const
  {
    return &insert_;
  }

  void setEnable(bool t)
  {
    UndoCommand::setEnable(t);
    delete_.setEnable(t);
    insert_.setEnable(t);
  }

private:
  DeleteCommand delete_;
  InsertCommand insert_;
};
