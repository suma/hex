
#include "document.h"
#include "document_i.h"

Document::Document()
	: doc_(new DocumentImpl())
{
}

Document::~Document()
{
	delete doc_;
}

