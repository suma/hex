
#pragma once

#include <QUndoCommand>
#include <vector>
#include "document.h"


class InsertCommand : public QUndoCommand
{
public:
	InsertCommand(Document *doc, quint64 pos, const uchar *data, uint length, QUndoCommand *parent = 0);
	void undo();
	void redo();

private:
	Document *document_;
	quint64 position_;
	size_t offset_;
	uint length_;
};


class DeleteCommand : public QUndoCommand
{
public:
	DeleteCommand(Document *doc, quint64 pos, quint64 len, QUndoCommand *parent = 0);
	void undo();
	void redo();

private:
	Document *document_;
	quint64 position_;
	quint64 length_;
	std::vector<DocumentFragment> fragments_;
};



class ReplaceCommand : public QUndoCommand
{
public:
	ReplaceCommand(Document *doc, quint64 pos, quint64 len, const uchar *data, uint length, QUndoCommand *parent = 0);
	void undo();
	void redo();

private:
	DeleteCommand *delete_;
	InsertCommand *insert_;
};

