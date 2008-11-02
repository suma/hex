
#include <QtGui>
#include "hexview.h"

namespace Standard {

	namespace Color {
		enum color {
			Background = 0,
			Text,
		};
	}


////////////////////////////////////////
// Config
HexConfig::HexConfig()
	: Margin(10, 10, 10, 10)
	, ByteMargin(0, 0, 5, 5)
	, Font("times", 24)
	, FontMetrics(Font)
{
	Colors[0] = QColor(255,0,0);
	Colors[1] = QColor(0,0,255);

	for (int i = 1; i < Num; i++) {
		Spaces[i] = FontMetrics.maxWidth();
	}
	Spaces[0] = 0;
	Spaces[Num / 2] *= 2;

	calculate();
}

void HexConfig::calculate()
{
	x_[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_[i] = x_[i-1] + byteWidth() + Spaces[i];
	}

	top_ = Margin.top() + ByteMargin.top();
}

////////////////////////////////////////
// View
HexView::HexView(QWidget *parent, Document *doc)
	: ::View(parent, doc)
{
}

void HexView::refreshPixmap()
{
	pix_.fill(config_.Colors[Color::Background]);

	QPainter painter(&pix_);
	//painter.initFrom(this);

	// TODO: Optimizing drawing

	// draw lines
	int yCount = 5;
	int y = config_.top();
	for (int i = 0; i < yCount; i++) {
		//drawLine
		const int x = config_.x(i);
		// draw hex
		y += config_.byteHeight();
	}

	update();
}


}	// namespace
