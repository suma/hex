
#include "document_i.h"


DocumentImpl::DocumentImpl()
	: length_(0)
{
}

DocumentImpl::~DocumentImpl()
{
}

uint DocumentImpl::insert_data(ulint pos, ulint bufPos, uint len, quint8 type)
{
    split(pos);
    uint x = documents.insert_single(pos, len);
    DocumentData *X = documents.fragment(x);
    X->type = type;
    X->bufferPosition = bufPos;

	length_ += len;

    uint w = documents.previous(x);
    if (w) {
        unite(w);
	}
	return w;
}

uint DocumentImpl::remove_data(ulint pos, ulint len)
{
    split(pos);
    split(pos+len);
	uint x = documents.findNode(pos);
    uint w = documents.previous(x);

	Q_ASSERT(x != 0);
	while (0 < len) {
		ulint size = documents.size(x);
		if (size < len) {
			len -= size;
			length_ -= size;
			x = documents.erase_single(x);
			x = documents.next(x);
		} else {
			length_ -= len;
			documents.setSize(x, size - len);
			break;
		}
	}

    if (w) {
        unite(w);
	}
	return w;
}

bool DocumentImpl::split(ulint pos)
{
    uint x = documents.findNode(pos);
    if (x) {
        ulint k = documents.position(x);
//          qDebug("found fragment with key %d, size_left=%d, size=%d to split at %d",
//                k, (*it)->size_left, (*it)->size, pos);
        if (k != pos) {
            Q_ASSERT(k <= pos);
            // need to resize the first fragment and add a new one
            DocumentData *X = documents.fragment(x);
            ulint oldsize = X->size;
            documents.setSize(x, pos-k);
            uint n = documents.insert_single(pos, oldsize-(pos-k));
            X = documents.fragment(x);
            DocumentData *N = documents.fragment(n);
            N->bufferPosition = X->bufferPosition + pos-k;
            N->type = X->type;
            return true;
        }
    }
    return false;
}


bool DocumentImpl::unite(uint f)
{
    uint n = documents.next(f);
    if (!n)
        return false;

    DocumentData *ff = documents.fragment(f);
    DocumentData *nf = documents.fragment(n);

    if (nf->type == ff->type && (ff->bufferPosition + ff->size == nf->bufferPosition)) {
        //if (isValidBlockSeparator(text.at(ff->stringPosition))
        //    || isValidBlockSeparator(text.at(nf->stringPosition)))
        //    return false;
        documents.setSize(f, ff->size + nf->size);
        documents.erase_single(n);
        return true;
    }
    return false;
}

