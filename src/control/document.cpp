
#include <QFile>
#include "document.h"
#include "document_i.h"


enum {
	DOCTYPE_BUFFER = 0,
};

Document::Document()
	: impl_(new DocumentImpl())
	, file_(NULL)
{
	buffer_.resize(1024 * 256);
}

Document::~Document()
{
	delete impl_;
	delete file_;
}

quint64 Document::length() const
{
	return impl_->length();
}

void Document::get(quint64 pos, uchar *buf, uint len) const
{
	Q_ASSERT(pos <= length());
	Q_ASSERT(len <= length());
	Q_ASSERT(pos <= length() - len);

	uint x = impl_->documents_.findNode(pos);

	Q_ASSERT(x != 0);
	const quint64 diff = pos - impl_->documents_.position(x);
	if (diff) {
		const quint64 fragmentSize = impl_->documents_.size(x) - diff;
		DocumentData *X = impl_->documents_.fragment(x);
		if (fragmentSize < len) {
			copy(X->type, X->bufferPosition + diff, fragmentSize, buf);
			len -= fragmentSize;
			buf += fragmentSize;
			x = impl_->documents_.next(x);
		} else {
			copy(X->type, X->bufferPosition + diff, len, buf);
			return;
		}
	}

	Q_ASSERT(x != 0);
	while (0 < len) {
		const quint64 fragmentSize = impl_->documents_.size(x);
		const uint cpSize = (static_cast<quint64>(len) < fragmentSize) ? len : static_cast<uint>(fragmentSize);
		const DocumentData *X = impl_->documents_.fragment(x);
		copy(X->type, X->bufferPosition, cpSize, buf);
		len -= cpSize;
		buf += cpSize;
		x = impl_->documents_.next(x);
	}
}

void Document::copy(uint type, quint64 pos, quint64 len, uchar *buf) const
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

	const quint64 bufPos = buffer_.size();
	buffer_.insert(buffer_.end(), buf, buf + len);
	impl_->insert_data(pos, bufPos, len, DOCTYPE_BUFFER);
}

void Document::remove(quint64 pos, quint64 len)
{
	Q_ASSERT(pos <= length());
	Q_ASSERT(len <= length());
	Q_ASSERT(pos <= length() - len);
	// TODO: collect QFile remove pieces
	impl_->remove_data(pos, len);
}




