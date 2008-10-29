
#include <QtGui>
#include "hexview.h"

namespace Standard {

// Config
HexConfig::HexConfig()
	: Margin(10, 10, 10, 10)
	, ByteMargin(0, 0, 5, 5)
	, Font("times", 24)
	, FontMetrics(Font)
{
	Colors[0] = QColor(255,0,0);
	Colors[1] = QColor(0,0,255);

	calculate();
}

void HexConfig::calculate()
{
}

// View
HexView::HexView(QWidget *parent, Document *doc)
	: ::View(parent, doc)
{
}

void HexView::refreshPixmap()
{
	pix_.fill(getConfig().Colors[Color::Background]);

	QPainter painter(&pix_);
	//painter.initFrom(this);

	update();
}

}	// namespace
