
#include <QtGui>
#include <algorithm>
#include <vector>
#include "hexview.h"
#include "scursor.h"
#include "../document.h"
#include "../highlight.h"

using namespace std;

namespace Standard {

////////////////////////////////////////
// Config
HexConfig::HexConfig()
	: Margin(10, 10, 10, 10)
	, ByteMargin(0, 4, 5, 1)
	, Font("times", 24)
	, FontMetrics(Font)
{
	Colors[Color::Background] = QColor(255,0,0);
	Colors[Color::Text] = QColor(0,0,255);

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
HexView::HexView(QWidget *parent, Document *doc, Cursor *cur, Highlight *hi)
	: ::View(parent, doc, hi)
	, cur_(cur)
{
	setWindowOpacity(0.5);
}

void HexView::refreshPixmap()
{
	refreshPixmap(0);
}

void HexView::refreshPixmap(int)
{
	//
	// TODO: Optimizing drawing

	pix_.fill(config_.Colors[Color::Background]);
	QPainter painter(&pix_);
	//painter.initFrom(this);

	painter.drawText(20, 20, QString("abcdefg"));

	/* sel.end < vpostop => not selected
	 * vposend < sel.begin => not selected
	 *
	 * 4 patterns
	 * begin >= top  :: 最初から
	 * begin != top  :: 途中から
	 * end <= bottom :: 最後まで
	 * end != bottom :: 途中まで
	 *
	 */

	if (!doc_->length()) {
		// draw Empty Background only
		return;
	}

	// Calculate drawing area
	int y = config_.top();
	const int yMax = height();
	const int yCount = (height() - y + config_.byteHeight()) / config_.byteHeight();
	quint64 top = cur_->Top * 16;
	const uint size = min(doc_->length() - top, 16ULL * yCount);

	// Calculate selectead area
	const int xb = 0, xe = width();
	bool selected = false;
	quint64 sb = 0, se = 0;
	if (cur_->Selected) {
		sb = max(cur_->SelBegin, cur_->SelEnd);
		se = min(cur_->SelBegin, cur_->SelEnd);
		if (top <= se) {
			const quint64 vpos_end = max(top + (16ULL * yCount), top + size);
			if (sb <= vpos_end) {
				selected = true;
			}
		}
	}

	// Copy data
	if (buff_.capacity() < size) {
		buff_.reserve(size);
	}
	doc_->get(top, &buff_[0], size);

	// TODO: Adding cache class for calculated values if these processing is bottle neck
	::DrawInfo di(y, top, yCount, xb, xe, sb, se, size, selected);
	getDrawColors(di, colors_, config_.Colors);

	// draw

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
