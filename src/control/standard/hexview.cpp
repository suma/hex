
#include <QtGui>
#include <algorithm>
#include "hexview.h"

using namespace std;

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
	Spaces[0] = Spaces[Num] = 0;
	Spaces[Num / 2] *= 2;

	calculate();
}

void HexConfig::calculate()
{
	// Pos
	x_[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_[i] = x_[i-1] + byteWidth() + Spaces[i];
	}

	// Area
	xarea_[0] = Margin.left();
	for (int i = 1; i < Num; i++) {
		xarea_[i] = xarea_[i-1] + byteWidth() + Spaces[i];
	}
	for (int i = 1; i < Num; i++) {
		xarea_[i] -= Spaces[i] / 2;
	}

	top_ = Margin.top() + ByteMargin.top();
}

int HexConfig::toPos(int x)
{
	if (x < Margin.left()) {
		return -1;
	}

	int a = (int)distance(xarea_, lower_bound(xarea_, xarea_ + Num + 1, x));
	if (a == Num + 1) {
		return 32;
	}

	if (x <= xarea_[a] + FontMetrics.maxWidth()) {
		return x * 2 + 1;
	}
	return x * 2;
}

int HexConfig::toLine(int y)
{
}

////////////////////////////////////////
// View
HexView::HexView(QWidget *parent, Document *doc)
	: ::View(parent, doc)
{
	setWindowOpacity(0.5);
}

void HexView::refreshPixmap()
{
	refreshPixmap(0);
}

void HexView::refreshPixmap(int)
{
	pix_.fill(config_.Colors[Color::Background]);

	QPainter painter(&pix_);
	//painter.initFrom(this);

	// TODO: Optimizing drawing
	painter.drawText(20, 20, QString("abcdefg"));

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

void HexView::mousePressEvent(QMouseEvent*)
{
}

void HexView::mouseMoveEvent(QMouseEvent*)
{
}

void HexView::mouseReleaseEvent(QMouseEvent*)
{
}


}	// namespace
