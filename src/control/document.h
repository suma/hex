#ifndef DOCUMENT_H_INC
#define DOCUMENT_H_INC

#include <QString>
#include <vector>

class DocumentImpl;
class QFile;

class DocumentOriginal
{
public:
	virtual quint64 length() const = 0;
	virtual void get(quint64 pos, uchar *buf, quint64 len) const = 0;
};

class Document
{
public:
	// 空で作成
	Document();
	// ファイルから開く
	Document(QFile *file);
	// 既存のドキュメントをコピー
	Document(const Document &doc, bool writemode);
	virtual ~Document();


	// immutable methods
	quint64 length() const;
	void get(quint64 pos, uchar *buf, uint len) const;


	// mutable methods
	void insert(quint64 pos, const uchar *buf, uint len);
	void remove(quint64 pos, quint64 len);

	// TODO: undo/redo


	// file method
	//  save, saveAs, ....

private:
	void copy(uint type, quint64 pos, quint64 len, uchar *buf) const;


protected:
	DocumentImpl *impl_;
	DocumentOriginal *original_;
	QFile *file_;
	std::vector<uchar> buffer_;

};


#endif

