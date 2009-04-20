
#include <QFile>
#include "document.h"
#include "document_i.h"


enum {
	DOCTYPE_BUFFER = 0,
};

Document::Document()
	: document(new DocumentImpl())
	, file(NULL)
{
	buffer.reserve(1024 * 256);
}

Document::~Document()
{
	delete document;
	delete file;
}

quint64 Document::length() const
{
	return document->length();
}

void Document::get(quint64 pos, uchar *buf, uint len)
{
	Q_ASSERT(pos <= length());
	Q_ASSERT(len <= length());
	Q_ASSERT(pos <= length() - len);

	uint x = document->documents.findNode(pos);

	Q_ASSERT(x != 0);
	quint64 diff = pos - document->documents.position(x);
	if (diff) {
		quint64 fragmentSize = document->documents.size(x) - diff;
		DocumentData *X = document->documents.fragment(x);
		if (fragmentSize < len) {
			copy(X->type, X->bufferPosition + diff, fragmentSize, buf);
			len -= fragmentSize;
			buf += fragmentSize;
			x = document->documents.next(x);
		} else {
			copy(X->type, X->bufferPosition + diff, len, buf);
			return;
		}
	}

	Q_ASSERT(x != 0);
	while (0 < len) {
		quint64 fragmentSize = document->documents.size(x);
		DocumentData *X = document->documents.fragment(x);
		if (fragmentSize < len) {
			copy(X->type, X->bufferPosition, fragmentSize, buf);
			len -= fragmentSize;
			buf += fragmentSize;
			x = document->documents.next(x);
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
		Q_ASSERT(pos <= buffer.size());
		Q_ASSERT(len <= buffer.size());
		Q_ASSERT(pos <= buffer.size() - len);
		memcpy(buf, &buffer[static_cast<uint>(pos)], len);
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

	const quint64 bufPos = buffer.size();
	buffer.insert(buffer.end(), buf, buf + len);
	document->insert_data(pos, bufPos, len, DOCTYPE_BUFFER);
}

void Document::remove(quint64 pos, quint64 len)
{
	Q_ASSERT(pos <= length());
	Q_ASSERT(len <= length());
	Q_ASSERT(pos <= length() - len);
	document->remove_data(pos, len);
}




