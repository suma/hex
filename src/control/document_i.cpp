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
/* original by src/gui/text/qtextdocument_p.cpp */

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
	//qDebug("remove_data pos:%llu len:%llu", pos, len);
    split(pos);
    split(pos+len);
	uint x = documents_.findNode(pos);
    uint w = documents_.previous(x);

	Q_ASSERT(x != 0);
	while (0 < len) {
		ulint size = documents_.size(x);
		if (size < len) {
			//qDebug("pos:%llu x:%u size:%llu len:%llu", pos, x, size, len);
			len -= size;
			length_ -= size;
			x = documents_.erase_single(x);
			//qDebug("erased x:%u", x);
			if (!x) {
				x = documents_.findNode(pos);
			} else {
				x = documents_.next(x);
			}
			//qDebug("next  x:%u", x);
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
            ulint oldsize = X->size_array[0];
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

    if (nf->type == ff->type && (ff->bufferPosition + ff->size_array[0] == nf->bufferPosition)) {
        //if (isValidBlockSeparator(text.at(ff->stringPosition))
        //    || isValidBlockSeparator(text.at(nf->stringPosition)))
        //    return false;
        documents_.setSize(f, ff->size_array[0] + nf->size_array[0]);
        documents_.erase_single(n);
        return true;
    }
    return false;
}

