
#include "view.h"
#include "caretdrawer.h"
#include "../document.h"

namespace Standard {

View::View(QWidget *parent, Document *doc)
	: QWidget(parent)
	, document_(doc)
{
}

View::~View()
{
}

void View::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, pix_);

	caretDrawEvent(&painter);
}

void View::resizeEvent(QResizeEvent *rs)
{
	if (pix_.size().width() < rs->size().width() ||
		pix_.size().height() < rs->size().height()) {
		pix_ = QPixmap(rs->size());
	}
}


}	// namespace

