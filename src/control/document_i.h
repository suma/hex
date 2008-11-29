#ifndef DOCUMENT_I_H_INC
#define DOCUMENT_I_H_INC

#include "qfragmentmap_p.h"

class DocumentData : public QFragment
{
public:
	void initialize() {}
	void invalidate() const {}
	void free() {}
	quint64 bufferPosition;
	quint8 type;
};


class DocumentImpl
{
public:

	typedef QFragmentMap<DocumentData> DocumentMap;
	typedef DocumentMap::ConstIterator DocumentIterator;

public:
	DocumentImpl();
	~DocumentImpl();

	uint insert_data(ulint pos, ulint bufPos, uint length, quint8 type);
	uint remove_data(ulint pos, ulint length);
	bool split(ulint pos);
	bool unite(uint f);

	inline ulint length() const { return length_; }

protected:
	ulint length_;
	DocumentMap documents_;

};


#endif

