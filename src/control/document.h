#ifndef DOCUMENT_H_INC
#define DOCUMENT_H_INC

#include <QString>
#include <vector>

class DocumentImpl;
class QFile;


class Document
{
public:
	Document();
	Document(const QString &str, bool writemode);
	Document(const Document &doc, bool writemode);
	virtual ~Document();

	quint64 length() const;

	void get(quint64 pos, uchar *buf, uint len);
	void insert(quint64 pos, const uchar *buf, uint len);
	void remove(quint64 pos, quint64 len);

private:
	void copy(uint type, quint64 pos, quint64 len, uchar *buf);


protected:
	DocumentImpl *impl_;
	QFile *file_;
	std::vector<uchar> buffer_;

};


#endif
