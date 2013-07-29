/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

// This file is derived by
// http://qt.gitorious.org/qt/qt/blobs/raw/4.8/src/gui/text/qtextdocument_p.cpp

#include "document_i.h"

DocumentImpl::DocumentImpl()
{
}

DocumentImpl::~DocumentImpl()
{
}

uint DocumentImpl::insert_data(ulint pos, ulint bufPos, ulint length, quint8 type)
{
    Q_ASSERT(length != 0);

    split(pos);
    uint x = documents_.insert_single(pos, length);

    DocumentData *X = documents_.fragment(x);
    X->type = type;
    X->bufferPosition = bufPos;
    uint w = documents_.previous(x);
    if (w)
        unite(w);
    return w;
}

uint DocumentImpl::remove_data(qint64 pos, ulint length)
{
    Q_ASSERT(pos >= 0 && pos+length <= documents_.length());

    uint x = documents_.findNode(pos);
    return move(pos, -1, length);
}

uint DocumentImpl::move(qint64 pos, qint64 to, ulint length)
{
    Q_ASSERT(to <= documents_.length() && to <= pos);
    Q_ASSERT(pos >= 0 && pos+length <= documents_.length());

    if (pos == to)
        return 0;
    const bool needsInsert = to != -1;

    split(pos);
    split(pos+length);

    uint dst = needsInsert ? documents_.findNode(to) : 0;
    ulint dstPos = to;

    uint x = documents_.findNode(pos);
    uint end = documents_.findNode(pos+length);
    uint w = 0;
    while (x != end) {
        uint n = documents_.next(x);

        w = documents_.erase_single(x);

        if (needsInsert) {
            // TODO(suma): Write move test
            DocumentData *X = documents_.fragment(x);
            insert_data(dstPos, X->bufferPosition, X->size_array[0], X->type);
            dstPos += X->size_array[0];
        }

        x = n;
    }
    if (w)
        unite(w);

    return w;
}

bool DocumentImpl::split(ulint pos)
{
    uint x = documents_.findNode(pos);
    if (x) {
        ulint k = documents_.position(x);
//          qDebug("found fragment with key %d, size_left=%d, size=%d to split at %d",
//                k, (*it)->size_left[0], (*it)->size_array[0], pos);
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
