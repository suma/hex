
#include <QtGui>
#include <algorithm>
#include "view.h"
#include "document.h"

using namespace std;

View::View(QWidget *parent, Document *doc, Highlight *hi)
	: QWidget(parent)
	, document_(doc)
	, high_(hi)
{
}

View::~View()
{
}

void View::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, pix_);
}

void View::resizeEvent(QResizeEvent *rs)
{
	if (pix_.size().width() < rs->size().width() ||
		pix_.size().height() < rs->size().height()) {
		pix_ = QPixmap(rs->size());
	}
}


