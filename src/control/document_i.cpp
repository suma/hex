/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/
/* copied by src/gui/text/qtextdocument_p.cpp */

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
            ulint oldsize = X->size_array[0];
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

    if (nf->type == ff->type && (ff->bufferPosition + ff->size_array[0] == nf->bufferPosition)) {
        //if (isValidBlockSeparator(text.at(ff->stringPosition))
        //    || isValidBlockSeparator(text.at(nf->stringPosition)))
        //    return false;
        documents.setSize(f, ff->size_array[0] + nf->size_array[0]);
        documents.erase_single(n);
        return true;
    }
    return false;
}

