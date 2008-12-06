
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
	: Margin(8, 4, 10, 10)
	, ByteMargin(1, 1, 1, 2)
	//, Font("Courier", 13)
	, Font("Monaco", 13)
	, FontMetrics(Font)
{
	// Coloring
	Colors[Color::Background] = QColor(255,255,255);
	Colors[Color::Text] = QColor(0,0,0);
	Colors[Color::SelBackground] = QColor(0xCC,0xCC,0xFF);
	Colors[Color::SelText] = QColor(0,0x40,0x40);

	// Font
	Font.setFixedPitch(true);
	qDebug("isFixed: %d", Font.fixedPitch());

	calculate();
}

void HexConfig::calculate()
{
	// Spaces
	for (int i = 1; i < Num; i++) {
		Spaces[i] = charWidth();
	}
	Spaces[0] = Spaces[Num] = 0;
	Spaces[Num / 2] *= 2;

	// Pos
	x_[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_[i] = x_[i-1] + byteWidth() + Spaces[i];
	}

	// Pos of end
	for (int i = 0; i < Num; i++) {
		X_[i] = x_[i] + charWidth(2);
		qDebug("i:%d x: %d X: %d", i, x_[i], X_[i]);
	}

	// Area
	xarea_[0] = Margin.left();
	for (int i = 1; i < Num; i++) {
		xarea_[i] = xarea_[i-1] + byteWidth() + Spaces[i];
	}
	for (int i = 1; i < Num; i++) {
		//xarea_[i] -= Spaces[i] / 2;
		xarea_[i] -= Spaces[i];
	}
	xarea_[Num] = xarea_[Num-1] + byteWidth();

	top_ = Margin.top();
}

int HexConfig::drawableLines(int height)
{
	const int y = top() + byteMargin().top();
	return (height - y + byteHeight()) / byteHeight();
}

int HexConfig::XToPos(int x) const
{
	if (x < Margin.left()) {
		return -1;
	}

	return (int)distance(xarea_, lower_bound(xarea_, xarea_ + Num + 2, x)) - 1;
}

int HexConfig::YToLine(int y) const
{
	if (y < Margin.top()) {
		return -1;
	}
	return (y - Margin.top()) / byteHeight();
}

////////////////////////////////////////
// View
HexView::HexView(QWidget *parent, Document *doc, Cursor *cur, Highlight *hi)
	: ::View(parent, doc, hi)
	, cur_(cur)
{
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
	painter.setFont(config_.Font);

	if (!doc_->length()) {
		// TODO: draw Empty Background only
		return;
	}

	// Compute drawing area
	int yt = config_.top();
	int y = config_.top() + config_.byteMargin().top();
	const int yMax = height();
	const int yCount = config_.drawableLines(height());
	quint64 top = cur_->Top * 16;
	const uint size = min(doc_->length() - top, 16ULL * yCount);

	// Compute selectead area
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

	// TODO: Adding cache class for computed values if this function is bottle neck
	::DrawInfo di(y, top, yCount, xb, xe, sb, se, size, selected);
	getDrawColors(di, dcolors_, config_.Colors);

	// Draw lines
	DCIList::iterator itr = dcolors_.begin(), end = dcolors_.end();
	QBrush br;
	bool init_itr = false;
	for (int i = 0, j = 0, cont; itr != end;) {
		if (!init_itr) {
			// Create brush
			br = QBrush(itr->Colors[Color::Background]);

			// Set color
			painter.setBackground(br);
			painter.setPen(itr->Colors[Color::Text]);

			// ok
			init_itr = true;
		}

		// Continuous size
		cont = min((int)(itr->Length), HexConfig::Num - j);
		qDebug("itr->Length:%d j:%d cont:%d", itr->Length, j, cont);
		// Draw background
		int begin, width;
		if (2 <= cont) {
			begin = config_.x(j);
			width = config_.X(j + cont - 1) - begin;
		} else {
			begin = config_.x(j);
			width = config_.charWidth(2);
		}
		painter.fillRect(begin, yt, width, config_.byteHeight(), br);

		// Draw text
		for (int k = 0; k < cont; k++, i++, j++) {
			QString hex;
			byteToHex(buff_[i], hex);
			const int width = config_.charWidth(2);
			painter.drawText(config_.x(j), y, width, config_.byteHeight(), Qt::AlignCenter, hex);
		}
		qDebug("y: %d, yt:%d", y, yt);

		itr->Length -= cont;
		j = j & 0xF;
		if (itr->Length == 0) {
			++itr;
			init_itr = false;
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
	h.resize(2);
	const uchar H = (c >> 4) & 0xF;
	if (H <= 9) {
		h[0] = QChar('0' + H);
	} else {
		h[0] = QChar('A' + H - 10);
	}
	const uchar L = c & 0xF;
	if (L <= 9) {
		h[1] = QChar('0' + L);
	} else {
		h[1] = QChar('A' + L - 10);
	}
}

void HexView::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) {
		cur_->SelBegin = cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->Toggle = true;
		qDebug("mouse down begin:%lld", cur_->SelBegin);
	}
}

void HexView::mouseMoveEvent(QMouseEvent *ev)
{
	if (cur_->Toggle) {
		cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->refreshSelected();

		refreshPixmap();
	}
}

void HexView::mouseReleaseEvent(QMouseEvent *ev)
{
	if (cur_->Toggle) {
		quint64 oldBegin = cur_->SelBegin;
		quint64 oldEnd = cur_->SelEnd;

		cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->refreshSelected();
		cur_->Toggle = false;
		qDebug("mouse release begin:%lld end:%lld", cur_->SelBegin, cur_->SelEnd);

		refreshPixmap();
	}
}

#define MIN(a,b) ((a) < (b) ? (a) : (b))
quint64 HexView::moveByMouse(int xx, int yy)
{
	int x = config_.XToPos(xx);
	int y = config_.YToLine(yy);

	qDebug("press x:%d y:%d top: %lld", x, y, cur_->Top);

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		x = y = 0;
	}

	cur_->Position = MIN(cur_->Top + x + y * 16, doc_->length());
	qDebug("Position: %lld", cur_->Position);
	return cur_->Position;
}
#undef MIN


}	// namespace
