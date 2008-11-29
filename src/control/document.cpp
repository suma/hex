
#include <QFile>
#include "document.h"
#include "document_i.h"

Document::Document()
	: doc_(new DocumentImpl())
	, file_(NULL)
{
}

Document::~Document()
{
	delete doc_;
	delete file_;
}

