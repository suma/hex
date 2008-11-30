
#include <QtGui>
#include <algorithm>
#include <vector>
#include "hexview.h"
#include "scursor.h"
#include "../document.h"

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
HexView::HexView(QWidget *parent, Document *doc, Cursor *cur)
	: ::View(parent, doc)
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
		return;
	}

	int y = config_.top();
	const int yMax = height();
	const int yCount = (height() - y + config_.byteHeight()) / config_.byteHeight();
	quint64 top = cur_->Top * 16;
	const int xb = 0, xe = width();
	const uint size = min(doc_->length() - top, 16ULL * yCount);
	bool selected = false;
	quint64 sb, se;
	if (cur_->Selected) {
		if (cur_->SelEnd < cur_->SelBegin) {
			sb = cur_->SelBegin;
			se = cur_->SelEnd;
		} else {
			sb = cur_->SelEnd;
			se = cur_->SelBegin;
		}
		if (top <= se) {
			quint64 a = 16ULL;
			const quint64 vpos_end = max(top + (16ULL * yCount), top + size);
			if (sb <= vpos_end) {
				selected = true;
			}
		}
	}
	DrawInfo di(y, top, yCount, xb, xe, sb, se, size);
	if (selected) {
		drawSelected(di);
	} else {
		drawNoSelected(di);
	}

	// ALL
	// LINE
	// AFTER, line

	// draw lines
	/*
	int y = config_.top();
	const int yMax = height();
	const int yCount = (height() - y + config_.byteHeight()) / config_.byteHeight();
	Q_ASSERT(doc_->length() >= (cur_->Top * 16));
	const quint64 top = cur_->Top * 16;
	const uint size = min(doc_->length() - top, 16ULL * yCount);
	vector<uchar> buff(size);
	doc_->get(top, &buff[0], size);
	*/


	update();
}

// x: [xb, xe)
// y: [line, line_e)
// line: line begin
// line_e: line end
// xb: x begin
// xe: x end
void HexView::drawSelected(const DrawInfo &di)
{
}

void HexView::drawNoSelected(const DrawInfo &di)
{
	/*
	Q_ASSERT(doc_->length() >= (top * 16));
	const uint size = min(doc_->length() - top, 16ULL * count);
	vector<uchar> buff(size);
	doc_->get(top, &buff[0], size);
	*/

	//for (int i = 0; i < count; i++) {
	//}
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
