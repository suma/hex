
#include <QtGui>
#include "addressview.h"
#include "scursor.h"

namespace Standard {

AddressView::AddressView(QWidget *parent, Document *doc, Cursor *cur)
	: ::View(parent, doc)
	, cursor(cur)
{
}

void AddressView::refreshPixmap()
{
	pix_.fill(QColor(0,255,0));

	QPainter painter(&pix_);
	//painter.initFrom(this);

	update();
}


}	// namespace

