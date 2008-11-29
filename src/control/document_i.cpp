
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
    uint x = documents_.insert_single(pos, len);
    DocumentData *X = documents_.fragment(x);
    X->type = type;
    X->bufferPosition = bufPos;

	length_ += len;

    uint w = documents_.previous(x);
    if (w) {
        unite(w);
	}
	return w;
}

uint DocumentImpl::remove_data(ulint pos, ulint len)
{
    split(pos);
    split(pos+len);
	uint x = documents_.findNode(pos);
    uint w = documents_.previous(x);

	Q_ASSERT(x != 0);
	while (0 < len) {
		ulint size = documents_.size(x);
		if (size < len) {
			len -= size;
			length_ -= size;
			x = documents_.erase_single(x);
			x = documents_.next(x);
		} else {
			length_ -= len;
			documents_.setSize(x, size - len);
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
    uint x = documents_.findNode(pos);
    if (x) {
        ulint k = documents_.position(x);
//          qDebug("found fragment with key %d, size_left=%d, size=%d to split at %d",
//                k, (*it)->size_left, (*it)->size, pos);
        if (k != pos) {
            Q_ASSERT(k <= pos);
            // need to resize the first fragment and add a new one
            DocumentData *X = documents_.fragment(x);
            ulint oldsize = X->size;
            documents_.setSize(x, pos-k);
            uint n = documents_.insert_single(pos, oldsize-(pos-k));
            X = documents_.fragment(x);
            DocumentData *N = documents_.fragment(n);
            N->bufferPosition = X->bufferPosition + pos-k;
            N->type = X->type;
            return true;
        }
    }
    return false;
}


bool DocumentImpl::unite(uint f)
{
    uint n = documents_.next(f);
    if (!n)
        return false;

    DocumentData *ff = documents_.fragment(f);
    DocumentData *nf = documents_.fragment(n);

    if (nf->type == ff->type && (ff->bufferPosition + ff->size == nf->bufferPosition)) {
        //if (isValidBlockSeparator(text.at(ff->stringPosition))
        //    || isValidBlockSeparator(text.at(nf->stringPosition)))
        //    return false;
        documents_.setSize(f, ff->size + nf->size);
        documents_.erase_single(n);
        return true;
    }
    return false;
}

