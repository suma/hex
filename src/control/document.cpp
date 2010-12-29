
#include <limits>
#include <QFile>
#include <QDataStream>
#include <QUndoStack>
#include "document.h"
#include "filemapreader.h"

const size_t Document::DEFAULT_BUFFER_SIZE = 0x100000;


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
	size_t buffer_size_;

	QFile *file_;

	// pointer to mapped
	mutable uchar *ptr_;

	// mapped offset and size
	mutable quint64 offset_;
	mutable quint64 size_;
	

public:
	FileOriginal(QFile *file, size_t buffer_size = Document::DEFAULT_BUFFER_SIZE)
		: buffer_size_(buffer_size)		// buffer_size = 0x10 ^ N
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
		
		if (length() > 0) {
			remap(0);
		}
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
		
			// start position
			const uint start = static_cast<uint>(pos & (buffer_size_ - 1));
			// copy size
			const uint copy_size = qMin((quint64)size_ - start, len);

			// copy
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

		size_ = qMin(length() - offset_, (quint64)buffer_size_);
		ptr_ = file_->map(offset_, size_);
	}

	bool isOffsetCovered(quint64 pos) const
	{
		Q_ASSERT(pos <= length());
		return offset_ <= pos && pos - offset_ <= size_;
	}
};

bool Document::WriteCallback::writeCallback(quint64)
{
	return true;
}

void Document::WriteCallback::writeCompleted()
{
}

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
	, undo_stack_(new QUndoStack(this))
{
}

Document::Document(QFile *file)
	: impl_(new DocumentImpl())
	, original_(new FileOriginal(file))
	, file_(file)
	, undo_stack_(new QUndoStack(this))
{
	impl_->insert_data(0, 0, file->size(), DOCTYPE_ORIGINAL);
}


Document::Document(QFile *file, uint buffer_size)
	: impl_(new DocumentImpl())
	, original_(new FileOriginal(file, buffer_size))
	, file_(file)
	, undo_stack_(new QUndoStack(this))
{
	impl_->insert_data(0, 0, file->size(), DOCTYPE_ORIGINAL);
}

Document::~Document()
{
	delete undo_stack_;
	delete impl_;
	delete original_;
		// file_ deleted in original_
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
	get(pos, len, FragmentCopier(this, buf));
}

Document::FragmentList Document::get(quint64 pos, quint64 len) const
{
	FragmentList fragments;
	if (len == 0) {
		len = length();
	}
	get(pos, len, std::back_inserter(fragments));
	return fragments;
}

void Document::copy(quint8 type, quint64 pos, quint64 len, uchar *buf) const
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
	emit inserted(pos, static_cast<quint64>(len));
}

void Document::insert(quint64 pos, DocumentFragment fragment)
{
	Q_ASSERT(pos <= length());
	Q_ASSERT(fragment.length() != 0);
	impl_->insert_data(pos, fragment.position(), fragment.length(), fragment.type());
	emit inserted(pos, fragment.length());
}

void Document::remove(quint64 pos, quint64 len)
{
	Q_ASSERT(pos <= length());
	Q_ASSERT(len <= length());
	Q_ASSERT(pos <= length() - len);
	impl_->remove_data(pos, len);
	emit removed(pos, len);
}

Document::Buffer &Document::buffer()
{
	return buffer_;
}

QFile *Document::file() const
{
	return file_;
}

QUndoStack *Document::undoStack() const
{
	return undo_stack_;
}

Document *Document::reopenKeepUndo(Document *doc, size_t max_buffer)
{
	// TODO: implement
	return NULL;
}

bool Document::overwritable() const
{
	if (file_ == NULL) {
		return false;
	}

	FragmentList fragments(get());

	// check overwrite
	quint64 index = 0;
	FragmentList::const_iterator it = fragments.begin(), end = fragments.end();
	while (it != end) {
		if (it->type() == DOCTYPE_ORIGINAL) {
			if (index != it->position()) {
				// impossible
				return false;
			}
		}
		index += it->length();
		++it;
	}

	return true;
}

bool Document::write(WriteCallback *callback)
{
	Q_ASSERT(overwritable() == true);

	FragmentList fragments(get());

	// get overwrite size
	quint64 index = 0;
	quint64 write_size = 0;
	{
		FragmentList::const_iterator it = fragments.begin();
		while (it != fragments.end()) {
			if (it->type() == DOCTYPE_BUFFER) {
				write_size += it->length();
			} else if (it->type() == DOCTYPE_ORIGINAL) {
				if (index != it->position()) {
					// impossible
					return false;
				}
			}

			index += it->length();
			++it;
		}
	}

	if (callback != NULL) {
		//callback->setWrite(write_size);
	}

	// TODO: check file_->name() can be written

	const uint BLOCK_SIZE = 1024 * 1024 * 4;
	index = 0;
	quint64 completed = 0;
	QDataStream outStream(file_);

	// piece table
	FragmentList::const_iterator it = fragments.begin(), end = fragments.end();
	while (it != end) {
		uchar *data = NULL;
		if (it->type() == DOCTYPE_BUFFER) {
			if (!file_->seek(index)) {
				goto label_error;
			}
			data = &buffer_[static_cast<uint>(it->position())];

			// write from memory(DOCTYPE_BUFFER) only
			quint64 piece_length = it->length();
			Q_ASSERT(piece_length != 0);
			while (true) {
				const size_t copy_size = static_cast<size_t>(qMin((quint64)BLOCK_SIZE, piece_length));
				const int res = outStream.writeRawData(reinterpret_cast<char*>(data), copy_size);
				if (res == -1) {
					goto label_error;
				}
				const size_t wrote_size = static_cast<size_t>(res);
				Q_ASSERT(wrote_size <= copy_size);

				// increment blocks
				completed += wrote_size;
				piece_length -= wrote_size;

				if (piece_length == 0) {
					break;
				}

				// callback
				if (callback != NULL) {
					callback->writeCallback(completed);
					// cancel unsupported
				}

				// iterate pointer
				data += wrote_size;
			}
		}
		++it;
	}


	if (callback != NULL) {
		callback->writeCompleted();
	}

	return true;

label_error:
	return false;
}

bool Document::write(QFile *out, WriteCallback *callback)
{
	Q_ASSERT(out != NULL);
	return write(0, length(), out, callback);
}

bool Document::write(quint64 pos, quint64 len, QFile *out, WriteCallback *callback)
{
	Q_ASSERT(out != NULL);
	Q_ASSERT(pos <= length());
	Q_ASSERT(len <= length());
	Q_ASSERT(pos <= length() - len);
	const uint BLOCK_SIZE = 1024 * 1024 * 4;

	// TODO: check file_->name() can be written

	if (!out->seek(0)) {
		return false;
	}

	if (len == 0) {
		return out->resize(0);
	}

	quint64 completed = 0;

	FileMapReader *reader = NULL;
	if (file_ != NULL) {
		reader = new FileMapReader(file_, BLOCK_SIZE);	// FIXME: BLOCK_SIZE
	}

	QDataStream outStream(out);
	FragmentList fragments = get(pos, len);

	// piece table
	FragmentList::const_iterator it = fragments.begin(), end = fragments.end();
	while (it != end) {
		uchar *data = NULL;
		if (it->type() == DOCTYPE_BUFFER) {
			data = &buffer_[static_cast<uint>(it->position())];
		} else if (it->type() == DOCTYPE_ORIGINAL) {
			reader->seek(it->position());
			data = reader->get();	// zero copy
		}

		// write piece buffer
		quint64 piece_length = it->length();
		Q_ASSERT(piece_length != 0);
		while (true) {
			const size_t copy_size = static_cast<size_t>(qMin((quint64)BLOCK_SIZE, piece_length));

			// write block
			Q_ASSERT(data != NULL);
			const int res = outStream.writeRawData(reinterpret_cast<char*>(data), copy_size);
			if (res == -1) {
				goto label_error;
			}
			const size_t wrote_size = static_cast<size_t>(res);
			Q_ASSERT(wrote_size <= copy_size);

			// increment blocks
			completed += wrote_size;
			piece_length -= wrote_size;

			if (piece_length == 0) {
				break;
			}

			// callback
			if (callback != NULL && !callback->writeCallback(completed)) {
				goto label_cancel;
			}

			// iterate pointer
			if (it->type() == DOCTYPE_BUFFER) {
				data += wrote_size;
			} else if (it->type() == DOCTYPE_ORIGINAL) {
				*reader += wrote_size;	// remap file if need
				data = reader->get();	// zero copy
			}
		}
		++it;
	}

	if (!out->resize(len)) {
		goto label_error;
	}

	if (callback != NULL) {
		callback->writeCompleted();
	}
	delete reader;


	// TODO: 
	//  case A: このままのバッファで編集を継続する（新規作成 or Originalの参照有りでも可）
	//  case B: 保存したバッファで編集をし直す（Documentを再構築）. UndoStackの修正が必須
	//
	//  case Aをデフォルト（write()）の動作として、再構築する(case Bの)場合は
	//   Editor側（Document作成する側）で制御すべき

	return true;

label_error:
	// handling error

	delete reader;
	return false;

label_cancel:

	delete reader;

	// Delete file
	out->remove();
	return true;
}



