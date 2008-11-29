
#include <QFile>
#include "document.h"
#include "document_i.h"



enum {
	DOCTYPE_BUFFER = 0,
};

Document::Document()
	: doc_(new DocumentImpl())
	, file_(NULL)
{
}

Document::~Document()
{
	delete doc_;
	delete file_;
}

quint64 Document::length() const
{
	return doc_->length();
}

void Document::get(quint64 pos, uchar *buf, uint len)
{
	Q_ASSERT(pos <= length());
	Q_ASSERT(len <= length());
	Q_ASSERT(pos <= length() - len);

	uint x = doc_->documents_.findNode(pos);

	Q_ASSERT(x != 0);
	quint64 diff = pos - doc_->documents_.position(x);
	if (diff) {
		quint64 size = doc_->documents_.size(x) - diff;
		DocumentData *X = doc_->documents_.fragment(x);
		if (size < len) {
			copy(X->type, X->bufferPosition + diff, size, buf);
			len -= size;
			buf += size;
			x = doc_->documents_.next(x);
		} else {
			copy(X->type, X->bufferPosition + diff, len, buf);
			return;
		}
	}

	Q_ASSERT(x != 0);
	while (0 < len) {
		quint64 size = doc_->documents_.size(x);
		DocumentData *X = doc_->documents_.fragment(x);
		if (size < len) {
			copy(X->type, X->bufferPosition, size, buf);
			len -= size;
			buf += size;
			x = doc_->documents_.next(x);
		} else {
			copy(X->type, X->bufferPosition, len, buf);
			return;
		}
	}
}

void Document::copy(uint type, quint64 pos, quint64 len, uchar *buf)
{
	Q_ASSERT(buf != NULL);
	switch (type) {
	case DOCTYPE_BUFFER:
		Q_ASSERT(pos <= buffer_.size());
		Q_ASSERT(len <= buffer_.size());
		Q_ASSERT(pos <= buffer_.size() - len);
		memcpy(buf, &buffer_[static_cast<uint>(pos)], len);
		break;
	default:
		;
	}
}

void Document::insert(quint64 pos, const uchar *buf, uint len)
{
	Q_ASSERT(buf != NULL);
	Q_ASSERT(len != 0);
	Q_ASSERT(pos <= length());

	quint64 bufPos = buffer_.size();
	buffer_.insert(buffer_.end(), buf, buf + len);
	doc_->insert_data(pos, bufPos, len, DOCTYPE_BUFFER);
}

void Document::remove(quint64 pos, quint64 len)
{
	Q_ASSERT(pos < length());
	Q_ASSERT(len < length());
	Q_ASSERT(pos < length() - len);
	doc_->remove_data(pos, len);
}




