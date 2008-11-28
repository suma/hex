#ifndef DOCUMENT_H_INC
#define DOCUMENT_H_INC

#include "qfragmentmap_p.h"

class DocumentData : public QFragment
{
public:
	void initialize() {}
	void invalidate() const {}
	void free() {}
	quint8 type;
	quint32 bufferPosition;
};


class Document
{
public:

	typedef QFragmentMap<DocumentData> DocumentMap;
	typedef DocumentMap::ConstIterator DocumentIterator;

public:
	Document();
	virtual ~Document();

	uint insert_data(ulint pos, uint bufPos, uint length, quint8 type);
	uint remove_data(ulint pos, ulint length);
	bool split(ulint pos);
	bool unite(uint f);

protected:
	DocumentMap documents_;

};


#endif

