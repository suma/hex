

#include <QUndoStack>
#include <algorithm>
#include <iterator>
#include "commands.h"

UndoCommand::UndoCommand(QUndoCommand *parent)
	: QUndoCommand(parent)
	, enable_(true)
{
}

InsertCommand::InsertCommand(Document *doc, quint64 pos, const uchar *data, uint length, QUndoCommand *parent)
	: UndoCommand(parent)
	, document_(doc)
	, position_(pos)
	, fragment_(Document::DOCTYPE_BUFFER, doc->buffer().size(), static_cast<quint64>(length))

{
	Q_ASSERT(length != 0);
	Document::Buffer &buffer = doc->buffer();
	buffer.insert(buffer.end(), data, data + length);
}

InsertCommand::InsertCommand(Document *doc, const InsertCommand *cmd, QUndoCommand *parent)
	: UndoCommand(parent)
	, document_(doc)
	, position_(cmd->position())
	, fragment_(Document::DOCTYPE_ORIGINAL, cmd->position(), cmd->fragment().length())
{
	setEnable(false);
}

void InsertCommand::undo()
{
	if (enable())
		document_->remove(position_, fragment_.length());
}

void InsertCommand::redo()
{
	if (enable())
		document_->insert(position_, fragment_);
}


DeleteCommand::DeleteCommand(Document *doc, quint64 pos, quint64 len, QUndoCommand *parent)
	: UndoCommand(parent)
	, document_(doc)
	, position_(pos)
	, length_(len)
	, fragments_(doc->get(pos, len))
{
	Q_ASSERT(len != 0);
}

DeleteCommand::DeleteCommand(Document *doc, const DeleteCommand *cmd, QUndoCommand *parent)
	: UndoCommand(parent)
	, document_(doc)
	, position_(cmd->position())
	, length_(cmd->length())
{
	const Document::FragmentList &fragments = cmd->fragments();
	fragments_.reserve(fragments.size());

	Document::Buffer &buffer = doc->buffer();
	Document::FragmentList::const_iterator it = fragments.begin();

	size_t buf_end = buffer.size();
	while (it != fragments.end()) {
		size_t size = it->length();
		cmd->document_->copy(it->type(), it->position(), size, &buffer[buf_end]);
		fragments_.push_back(DocumentFragment(Document::DOCTYPE_BUFFER, buf_end, size));

		buf_end += size;
	}

	setEnable(false);
}

void DeleteCommand::undo()
{
	if (enable()) {
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
}

void DeleteCommand::redo()
{
	if (enable())
		document_->remove(position_, length_);
}


ReplaceCommand::ReplaceCommand(Document *doc, quint64 pos, quint64 len, const uchar *data, uint insert_length, QUndoCommand *parent)
	: UndoCommand(parent)
	, delete_(doc, pos, len, parent)
	, insert_(doc, pos, data, insert_length, parent)
{
	Q_ASSERT(len != 0);
	Q_ASSERT(insert_length != 0);
}

ReplaceCommand::ReplaceCommand(Document *doc, const ReplaceCommand *cmd, QUndoCommand *parent)
	: UndoCommand(parent)
	, delete_(doc, cmd->deleteCommand(), parent)
	, insert_(doc, cmd->insertCommand(), parent)
{
	setEnable(false);
}


void ReplaceCommand::undo()
{
	if (enable()) {
		insert_.undo();
		delete_.undo();
	}
}

void ReplaceCommand::redo()
{
	if (enable()) {
		delete_.redo();
		insert_.redo();
	}
}















