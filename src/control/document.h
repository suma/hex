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
	virtual void get(quint64 pos, uchar *buf, uint len) const = 0;
};

class Document
{
public:
	Document();
	Document(const QString &str, bool writemode);
	Document(const Document &doc, bool writemode);
	virtual ~Document();

	quint64 length() const;

	void get(quint64 pos, uchar *buf, uint len) const;
	void insert(quint64 pos, const uchar *buf, uint len);
	void remove(quint64 pos, quint64 len);

private:
	void copy(uint type, quint64 pos, quint64 len, uchar *buf) const;


protected:
	DocumentImpl *impl_;
	QFile *file_;
	std::vector<uchar> buffer_;

};


#endif
