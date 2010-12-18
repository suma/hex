

#include <QUndoStack>
#include <algorithm>
#include <iterator>
#include "commands.h"

InsertCommand::InsertCommand(Document *doc, quint64 pos, const uchar *data, uint length, QUndoCommand *parent)
	: QUndoCommand(parent)
	, document_(doc)
	, position_(pos)
	, fragment_(Document::DOCTYPE_BUFFER, doc->buffer().size(), static_cast<quint64>(length))

{
	Q_ASSERT(length != 0);
	Document::Buffer &buffer = doc->buffer();
	buffer.insert(buffer.end(), data, data + length);
}

void InsertCommand::undo()
{
	document_->remove(position_, fragment_.length());
}

void InsertCommand::redo()
{
	document_->insert(position_, fragment_);
}


DeleteCommand::DeleteCommand(Document *doc, quint64 pos, quint64 len, QUndoCommand *parent)
	: QUndoCommand(parent)
	, document_(doc)
	, position_(pos)
	, length_(len)
	, fragments_(doc->get(pos, len))
{
	Q_ASSERT(len != 0);
}

void DeleteCommand::undo()
{
	quint64 index = position_;
	Document::FragmentList::const_iterator it = fragments_.begin(), end = fragments_.end();
	while (it != end) {
		if (it->length() != 0) {
			document_->insert(index, *it);
			index += it->length();
		}
		++it;
	}
}

void DeleteCommand::redo()
{
	// delete
	document_->remove(position_, length_);
}


ReplaceCommand::ReplaceCommand(Document *doc, quint64 pos, quint64 len, const uchar *data, uint insert_length, QUndoCommand *parent)
	: QUndoCommand(parent)
	, delete_(doc, pos, len, parent)
	, insert_(doc, pos, data, insert_length, parent)
{
	Q_ASSERT(len != 0);
	Q_ASSERT(insert_length != 0);
}


void ReplaceCommand::undo()
{
	insert_.undo();
	delete_.undo();
}

void ReplaceCommand::redo()
{
	delete_.redo();
	insert_.redo();
}















