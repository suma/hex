
#include <QFile>
#include <limits>
#include "document.h"
#include "document_i.h"


enum {
	DOCTYPE_BUFFER = 0,
	DOCTYPE_ORIGINAL = 1,
};

class EmptyOriginal : public DocumentOriginal
{
public:
	quint64 length() const
	{
		return 0;
	}

	void get(quint64 pos, uchar *buf, quint64 len) const
	{
		// TODO: throw Exception
	}
};

class FileOriginal : public DocumentOriginal
{
protected:
	uint buffer_size_;

	QFile *file_;

	// pointer to mapped
	mutable uchar *ptr_;

	// mapped offset and size
	mutable quint64 offset_;
	mutable quint64 size_;
	
#define DEFAULT_BUFFER_SIZE 0x100000

public:
	FileOriginal(QFile *file, uint buffer_size = DEFAULT_BUFFER_SIZE)
		: buffer_size_(buffer_size)		// 必ず16のN乗にする
		, file_(file)
		, ptr_(NULL)
		, offset_(0)
		, size_(0)
	{
		// check buffer_size
		uint size;
		for (size = 1; size < 0x10000000; size <<= 1) {
			if (size == buffer_size) {
				break;
			}
		}
		if (size == 0x10000000) {
			buffer_size_ = (uint)DEFAULT_BUFFER_SIZE;
		}
		
		remap(0);
	}

	virtual ~FileOriginal()
	{
		delete file_;
	}

	quint64 length() const
	{
		return file_->size();
	}

	void get(quint64 pos, uchar *buf, quint64 len) const
	{
		Q_ASSERT(pos < length() && len < length() - pos);

		while (len > 0) {
			if (!isOffsetCovered(pos)) {
				remap(pos);
			}
		
			// get start position
			uint start = static_cast<uint>(pos & (buffer_size_ - 1));
			// get copy size
			uint copy_size = qMin((quint64)buffer_size_ - start, len);

			// copy to memory
			memcpy(buf, ptr_ + start, copy_size);

			pos += copy_size;
			len -= copy_size;
		}
	}
private:

	void remap(quint64 pos) const
	{
		Q_ASSERT(pos < length());

		if (ptr_ != NULL) {
			file_->unmap(ptr_);
		}

		offset_ = pos & ~(buffer_size_ - 1);

		const quint64 map_size = qMin(length() - offset_, (quint64)buffer_size_);
		ptr_ = file_->map(offset_, map_size, QFile::NoOptions);
	}

	bool isOffsetCovered(quint64 pos) const
	{
		Q_ASSERT(pos <= length() - size_);
		const quint64 len = length();
		return (offset_ <= pos && (pos >= len && offset_ < len - pos));
	}
};

Document::Document()
	: impl_(new DocumentImpl())
	, file_(NULL)
	, original_(new EmptyOriginal())
{
	buffer_.resize(1024 * 256);
}

Document::~Document()
{
	delete impl_;
	delete file_;
	delete original_;
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
		const uint copy_size = (static_cast<quint64>(len) < fragmentSize) ? len : static_cast<uint>(fragmentSize);
		const DocumentData *X = impl_->documents_.fragment(x);
		copy(X->type, X->bufferPosition, copy_size, buf);
		len -= copy_size;
		buf += copy_size;
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
	case DOCTYPE_ORIGINAL:
		Q_ASSERT(pos <= original_->length());
		Q_ASSERT(len <= original_->length());
		Q_ASSERT(pos <= original_->length() - len);
		original_->get(pos, buf, len);
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




