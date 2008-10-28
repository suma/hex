
#include <QtGui>
#include "hexview.h"

namespace Standard {

HexView::HexView(QWidget *parent, Document *doc)
	: ::View(parent, doc)
{
}

void HexView::refreshPixmap()
{
	pix_.fill(QColor(0,255,255));
	QPainter painter(&pix_);
	//painter.initFrom(this);

	update();
}

}	// namespace
