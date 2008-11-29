#ifndef DOCUMENT_H_INC
#define DOCUMENT_H_INC

#include <QString>

class DocumentImpl;
class QFile;

class Document
{
public:
	Document();
	Document(const QString &str, bool writemode);
	Document(const Document &doc, bool writemode);
	virtual ~Document();

protected:
	DocumentImpl *doc_;
	QFile *file_;

};


#endif
