#ifndef DOCUMENT_H_INC
#define DOCUMENT_H_INC

class DocumentImpl;

class Document
{
public:
	Document();
	virtual ~Document();

protected:
	DocumentImpl *doc_;

};


#endif
