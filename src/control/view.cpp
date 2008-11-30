
#include <QtGui>
#include "view.h"
#include "document.h"

View::View(QWidget *parent, Document *doc)
	: QWidget(parent)
	, doc_(doc)
{
}

void View::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, pix_);
}

void View::resizeEvent(QResizeEvent *)
{
	pix_ = QPixmap(size());
	refreshPixmap();
}

