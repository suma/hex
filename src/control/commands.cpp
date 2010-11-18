

#include <QUndoStack>
#include <algorithm>
#include <iterator>
#include "commands.h"

InsertCommand::InsertCommand(Document *doc, quint64 pos, const uchar *data, uint length, QUndoCommand *parent)
	: QUndoCommand(parent)
	, document_(doc)
	, fragment_(Document::DOCTYPE_BUFFER, doc->buffer().size(), static_cast<quint64>(length))

{
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
{
}

void DeleteCommand::undo()
{
	//std::for_each(first, last, function)
	std::vector<DocumentFragment>::iterator it = fragments_.begin();
	quint64 index = position_;
	while (it != fragments_.end()) {
		document_->insert(index, *it);
		index += it->length();
	}
}

void DeleteCommand::redo()
{
	// delete
	document_->get(position_, length_, std::back_inserter(fragments_));
	document_->remove(position_, length_);
}


ReplaceCommand::ReplaceCommand(Document *doc, quint64 pos, quint64 len, const uchar *data, uint length, QUndoCommand *parent)
	: QUndoCommand(parent)
	, delete_(new DeleteCommand(doc, pos, len, parent))
	, insert_(new InsertCommand(doc, pos, data, length, parent))
{
}


void ReplaceCommand::undo()
{
	insert_->undo();
	delete_->undo();
}

void ReplaceCommand::redo()
{
	delete_->redo();
	insert_->redo();
}















