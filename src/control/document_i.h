#ifndef DOCUMENT_I_H_INC
#define DOCUMENT_I_H_INC

#include "qfragmentmap_p.h"

class DocumentData : public QFragment<1>
{
public:
	inline void initialize() {}
	inline void invalidate() const {}
	inline void free() {}
	quint64 bufferPosition;
	quint8 type;
};


class DocumentImpl
{
	friend class Document;
public:

	typedef QFragmentMap<DocumentData> DocumentMap;
	typedef DocumentMap::ConstIterator DocumentIterator;

public:
	DocumentImpl();
	~DocumentImpl();

	uint insert_data(ulint pos, ulint bufPos, ulint length, quint8 type);
	uint remove_data(qint64 pos, ulint length);
	uint move(qint64 pos, qint64 to, ulint length);
	bool split(ulint pos);
	bool unite(uint f);

	inline ulint length() const { return documents_.length(); }

protected:
	DocumentMap documents_;

};


#endif

