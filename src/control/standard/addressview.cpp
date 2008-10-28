
#include <QtGui>
#include "addressview.h"

namespace Standard {

AddressView::AddressView(QWidget *parent, Document *doc)
	: ::View(parent, doc)
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

