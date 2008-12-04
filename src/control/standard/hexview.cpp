
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
	, ByteMargin(0, 2, 2, 1)
	, Font("Monaco", 13)
	, FontMetrics(Font)
{
	Colors[Color::Background] = QColor(255,255,255);
	Colors[Color::Text] = QColor(0,0,0);
	Colors[Color::SelBackground] = QColor(0xCC,0xCC,0);
	Colors[Color::SelText] = QColor(0,0xCC,0xCC);

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

	top_ = Margin.top();
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

	//painter.drawText(220, 20, QString("abcdefg"));

	if (!doc_->length()) {
		// draw Empty Background only
		return;
	}

	// Calculate drawing area
	int yt = config_.top();
	int y = config_.top() + config_.byteMargin().top();
	const int yMax = height();
	const int yCount = (height() - y + config_.byteHeight()) / config_.byteHeight();
	quint64 top = cur_->Top * 16;
	const uint size = min(doc_->length() - top, 16ULL * yCount);

	// Calculate selectead area
	const int xb = 0, xe = width();
	bool selected = false;
	quint64 sb = 0, se = 0;
	if (cur_->Selected) {
		sb = min(cur_->SelBegin, cur_->SelEnd);
		se = max(cur_->SelBegin, cur_->SelEnd);
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

	// TODO: Adding cache class for calculated values if this function is bottle neck
	::DrawInfo di(y, top, yCount, xb, xe, sb, se, size, selected);
	getDrawColors(di, dcolors_, config_.Colors);

	// draw
	printf("y: %d , yt:%d\n", y, yt);
	DCIList::iterator itr = dcolors_.begin(), end = dcolors_.end();
	QBrush br;
	for (int i = 0, j = 0, m = 0, cont = 0; itr != end;) {
		if (m == 0) {
			br = QBrush(itr->Colors[Color::Background]);
			m = itr->Length;

			// Set color
			painter.setBackground(br);
			painter.setPen(itr->Colors[Color::Text]);
		}

		// Continuous size
		cont = min(m, HexConfig::Num - j);
		printf("m:%d j:%d cont:%d\n", m, j, cont);
		if (2 <= cont) {
			// Draw background
			Q_ASSERT(0 <= j && j < HexConfig::Num);
			Q_ASSERT(0 <= j+cont-1 && j+cont-1 < HexConfig::Num);
			painter.fillRect(config_.x(j), yt, config_.byteEnd(j+cont-1), yt + config_.byteHeight(), br);
		} else if (j < 15) { //(j < HexConfig::Num - 1) {
			Q_ASSERT(0 <= j && j < HexConfig::Num);
			Q_ASSERT(0 <= j+1 && j+1 < HexConfig::Num);
			painter.fillRect(config_.x(j), yt, config_.X(j+1), yt + config_.byteHeight(), br);
		}

		// Draw
		for (int k = 0; k < cont; k++, i++, j++) {
			QString hex;
			byteToHex(buff_[i], hex);
			painter.drawText(config_.x(j), y, hex);
		}

		m -= cont;
		j = j & 0xF;
		if (m == 0) {
			++itr;
		}
		if (j == 0) {
			y += config_.byteHeight();
			yt += config_.byteHeight();
		}
	}

	update();
}

void HexView::byteToHex(uchar c, QString &h)
{
	const uchar H = (c >> 4) & 0xF;
	if (H <= 9) {
		h = '0' + H;
	} else {
		h = 'A' + H;
	}
	const uchar L = c & 0xF;
	if (L <= 9) {
		h += '0' + L;
	} else {
		h += 'A' + L;
	}
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
