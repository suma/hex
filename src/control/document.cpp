
#include "document.h"

/*
static bool isValidBlockSeparator(const QChar &ch)
{
    return ch == QChar::ParagraphSeparator
        || ch == QTextBeginningOfFrame
        || ch == QTextEndOfFrame;
}
*/

Document::Document()
{
}

Document::~Document()
{
}

uint Document::insert_data(ulint pos, uint bufPos, uint length, quint8 type)
{
    split(pos);
    uint x = documents_.insert_single(pos, length);
    DocumentData *X = documents_.fragment(x);
    X->type = type;
    X->bufferPosition = bufPos;

    uint w = documents_.previous(x);
    if (w)
        unite(w);

	/*
    int b = blocks.findNode(pos);
    blocks.setSize(b, blocks.size(b)+length);

    Q_ASSERT(blocks.length() == fragments.length());

    QTextFrame *frame = qobject_cast<QTextFrame *>(objectForFormat(format));
    if (frame) {
        frame->d_func()->fragmentAdded(text.at(strPos), x);
        framesDirty = true;
    }

    adjustDocumentChangesAndCursors(pos, length, op);
	*/
	return w;
}

uint Document::remove_data(ulint pos, ulint length)
{
    split(pos);
	uint x = documents_.findNode(pos);
	while (x && 0 < length) {
		DocumentData *X = documents_.fragment(x);
		ulint size = X->size;
		if (length <= size) {
			length -= size;
			x = documents_.erase_single(x);
			x = documents_.next(x);	// which is more highspeed next or find?
		} else {
			size -= length;
			break;
		}
	}

    uint w = documents_.previous(documents_.findNode(pos));
    if (w)
        unite(w);


	/*
    QTextFrame *frame = qobject_cast<QTextFrame *>(objectForFormat(fragments.fragment(x)->format));
    if (frame) {
        frame->d_func()->fragmentRemoved(text.at(fragments.fragment(x)->stringPosition), x);
        framesDirty = true;
    }
	*/
	// collect erase data
	return w;
}

bool Document::split(ulint pos)
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


bool Document::unite(uint f)
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

