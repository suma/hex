
#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include "document_i.h"

class DocumentImpl;
class QFile;
class QUndoStack;

class DocumentFragment
{
private:
	quint8 type_;
	quint64 position_;
	quint64 length_;
public:
	DocumentFragment(quint8 type, quint64 pos, quint64 len)
		: type_(type)
		, position_(pos)
		, length_(len)
	{
	}

	quint8 type() const
	{
		return type_;
	}

	quint64 position() const
	{
		return position_;
	}

	quint64 length() const
	{
		return length_;
	}
};


class DocumentOriginal
{
public:
	DocumentOriginal();
	virtual ~DocumentOriginal();
	virtual quint64 length() const = 0;
	virtual void get(quint64 pos, uchar *buf, quint64 len) const = 0;
};


class Document : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Document)

public:
	typedef std::vector<uchar> Buffer;
	typedef std::vector<DocumentFragment> FragmentList;

	enum {
		DOCTYPE_BUFFER = 0,
		DOCTYPE_ORIGINAL = 1,
	};

	class WriteCallback
	{
	public:
		WriteCallback();
		virtual ~WriteCallback();
		virtual void writeStarted(quint64 max);
		virtual bool writeCallback(quint64 completed);
		virtual void writeCompleted();
	};

public:
	// 空で作成
	Document();
	// ファイルから開く
	Document(QFile *file);
	Document(QFile *file, uint buffer_size);

	Document *reopenKeepUndo( QFile *file, size_t max_buffer) const;
	// static Document * openFile(QFile*file);
	// static Document * create();

	virtual ~Document();


	//-- immutable methods

	// 
	quint64 length() const;
	bool isModified() const;

	// copy 
	void get(quint64 pos, uchar *buf, uint len) const;
	FragmentList get(quint64 pos = 0, quint64 len = 0) const;

	// copy DocumentFragment from piece table
	template <class T>
	T get(quint64 pos, quint64 len, T result) const
	{
		Q_ASSERT(len != 0);
		Q_ASSERT(pos <= length());
		Q_ASSERT(len <= length());
		Q_ASSERT(pos <= length() - len);

		uint x = impl_->documents_.findNode(pos);

		Q_ASSERT(x != 0);
		Q_ASSERT(impl_->documents_.size(x) != 0);
		const quint64 diff = pos - impl_->documents_.position(x);
		if (diff) {
			const quint64 copy_size = qMin(len, impl_->documents_.size(x) - diff);
			DocumentData *X = impl_->documents_.fragment(x);
			*result =  DocumentFragment(X->type, X->bufferPosition + diff, copy_size);
			if (copy_size == len) {
				// copy finished
				return result;
			}

			// next piece
			len -= copy_size;
			++result;
			x = impl_->documents_.next(x);
		}

		Q_ASSERT(x != 0);
		while (len != 0) {
			const quint64 fragmentSize = impl_->documents_.size(x);
			// skip zero-length piece
			if (fragmentSize != 0) {
				const quint64 copy_size = qMin(len, fragmentSize);
				const DocumentData *X = impl_->documents_.fragment(x);
				Q_ASSERT(copy_size != 0);
				*result = DocumentFragment(X->type, X->bufferPosition, copy_size);
				++result;

				len -= copy_size;
			}
			x = impl_->documents_.next(x);
		}

		return result;
	}

	// save
	bool overwritable() const;
	bool write(WriteCallback *callback);
	// saveas
	bool write(QFile *out, WriteCallback *callback);
	bool write(quint64 pos, quint64 len, QFile *out, WriteCallback *callback);

	//-- mutable methods
	void insert(quint64 pos, const uchar *buf, uint len);
	// file method
	//  save, saveAs, ....
	// 
	//

	QFile *file() const;
	QUndoStack *undoStack() const;
	Buffer &buffer();

public slots:
	//-- mutable methods
	void insert(quint64 pos, DocumentFragment fragment);
	void remove(quint64 pos, quint64 len);

signals:
	void inserted(quint64 pos, quint64 len);
	void removed(quint64 pos, quint64 len);
	void dataChanged();

public:
	const static size_t DEFAULT_BUFFER_SIZE;

	// ドキュメントの実体をバッファへコピーする
	void copy(quint8 type, quint64 pos, quint64 len, uchar *buf) const;
private:

	// UndoStack helper
	int findLimitIndex(size_t max_buffer) const;

	// Fragment Copy iterator
	class FragmentCopier;

	// Document Buffer Writer
	class BufferWriter;


protected:
	DocumentImpl *impl_;
	DocumentOriginal *original_;
	QFile *file_;
	QUndoStack *undo_stack_;
	Buffer buffer_;

};



