
#include <limits>
#include <QFile>
#include <QUndoStack>
#include "document.h"
#include "document_i.h"

enum {
	DOCTYPE_BUFFER = 0,
	DOCTYPE_ORIGINAL = 1,
};

const int Document::DEFAULT_BUFFER_SIZE = 0x100000;

class EmptyOriginal : public DocumentOriginal
{
public:
	quint64 length() const
	{
		return 0;
	}

	void get(quint64, uchar *, quint64) const
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
	

public:
	FileOriginal(QFile *file, uint buffer_size)
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
			buffer_size_ = (uint)Document::DEFAULT_BUFFER_SIZE;
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
		Q_ASSERT(pos <= length());
		Q_ASSERT(len <= length());
		Q_ASSERT(pos <= length() - len);

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

class Document::FragmentCopier
{
public:
	FragmentCopier(const Document *doc, uchar *buf)
		: document_(doc)
		, buf_(buf)
	{
	}
	FragmentCopier &operator=(DocumentFragment fragment)
	{
		document_->copy(fragment.type(), fragment.position(), fragment.length(), buf_);
		buf_ += fragment.length();
		return *this;
	}

	FragmentCopier &operator*()
	{
		return *this;
	}

	FragmentCopier &operator++()
	{
		return *this;
	}

	FragmentCopier &operator++(int)
	{
		return *this;
	}

private:
	const Document *document_;
	uchar *buf_;
};


Document::Document()
	: impl_(new DocumentImpl())
	, original_(new EmptyOriginal())
	, file_(NULL)
{
	buffer_.resize(1024 * 256);
}

Document::Document(QFile *file)
	: impl_(new DocumentImpl())
	, original_(new FileOriginal(file, DEFAULT_BUFFER_SIZE))
	, file_(file)
{
	impl_->insert_data(0, 0, file->size(), DOCTYPE_ORIGINAL);
}


Document::Document(QFile *file, uint buffer_size)
	: impl_(new DocumentImpl())
	, original_(new FileOriginal(file, buffer_size))
	, file_(file)
	, undo_stack_(new QUndoStack())
{
	impl_->insert_data(0, 0, file->size(), DOCTYPE_ORIGINAL);
}

Document::~Document()
{
	delete undo_stack_;
	delete file_;
	delete impl_;
	delete original_;
}

quint64 Document::length() const
{
	return impl_->length();
}

void Document::get(quint64 pos, uchar *buf, uint len) const
{
	get(pos, len, FragmentCopier(this, buf));
}

template <class T>
T Document::get(quint64 pos, quint64 len, T result) const
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
			*result =  DocumentFragment(X->type, X->bufferPosition + diff, fragmentSize);
			
			++result;
			len -= fragmentSize;
			x = impl_->documents_.next(x);
		} else {
			*result = DocumentFragment(X->type, X->bufferPosition + diff, len);
			return result;
		}
	}

	Q_ASSERT(x != 0);
	while (0 < len) {
		const quint64 fragmentSize = impl_->documents_.size(x);
		const uint copy_size = (static_cast<quint64>(len) < fragmentSize) ? len : static_cast<uint>(fragmentSize);
		const DocumentData *X = impl_->documents_.fragment(x);
		*result = DocumentFragment(X->type, X->bufferPosition, copy_size);
		++result;

		len -= copy_size;
		x = impl_->documents_.next(x);
	}

	return result;
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
	insert(pos, bufPos, len);
}

void Document::insert(quint64 pos, size_t offset, uint len)
{
	impl_->insert_data(pos, offset, len, DOCTYPE_BUFFER);
}

void Document::insert(quint64 pos, DocumentFragment fragment)
{
	impl_->insert_data(pos, fragment.position(), fragment.length(), fragment.type());
}

void Document::remove(quint64 pos, quint64 len)
{
	Q_ASSERT(pos <= length());
	Q_ASSERT(len <= length());
	Q_ASSERT(pos <= length() - len);
	impl_->remove_data(pos, len);
}

Document::Buffer &Document::buffer()
{
	return buffer_;
}

QUndoStack *Document::undoStack() const
{
	return undo_stack_;
}



