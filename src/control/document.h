
#pragma once

#include <QString>
#include <vector>

class DocumentImpl;
class QFile;
class QUndoStack;

class DocumentOriginal
{
public:
	virtual quint64 length() const = 0;
	virtual void get(quint64 pos, uchar *buf, quint64 len) const = 0;
};

class DocumentFragment
{
private:
	uint type_;
	quint64 position_;
	quint64 length_;
public:
	DocumentFragment(uint type, quint64 pos, quint64 len)
		: type_(type)
		, position_(pos)
		, length_(len)
	{
	}

	uint type() const
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

class Document
{
public:
	typedef std::vector<uchar> Buffer;

	// 空で作成
	Document();
	// ファイルから開く
	Document(QFile *file);
	Document(QFile *file, uint buffer_size);
	// 既存のドキュメントをコピー
	Document(const Document &doc, bool writemode);

	// TODO: static
	// static Document * openFile(QFile*file);
	// static Document * create();

	virtual ~Document();


	// immutable methods
	quint64 length() const;
	void get(quint64 pos, uchar *buf, uint len) const;

	template <class T>
	T get(quint64 pos, quint64 len, T t) const;


	// mutable methods
	void insert(quint64 pos, const uchar *buf, uint len);
	void insert(quint64 pos, size_t offset, uint len);
	void insert(quint64 pos, DocumentFragment fragment);
	void remove(quint64 pos, quint64 len);

	// TODO: undo/redo


	// file method
	//  save, saveAs, ....
	// 
	//

	QUndoStack *undoStack() const;
	Buffer &buffer();

public:
	const static int DEFAULT_BUFFER_SIZE;

private:
	void copy(uint type, quint64 pos, quint64 len, uchar *buf) const;
	class FragmentCopier;


protected:
	DocumentImpl *impl_;
	DocumentOriginal *original_;
	QFile *file_;
	QUndoStack *undo_stack_;
	Buffer buffer_;

};



