
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
	: Margin(3, 3, 3, 3)
	, ByteMargin(3, 1, 2, 2)
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
		X_[i] = x_[i] + charWidth(2) + ByteMargin.right();
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

int HexConfig::drawableLines(int height) const
{
	const int y = top() + byteMargin().top();
	return (height - y + byteHeight()) / byteHeight();
}

int HexConfig::drawableLinesN(int height) const
{
	return height / byteHeight() + 1;
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

enum {
	DRAW_ALL = 0,
	DRAW_LINE,
	DRAW_AFTER,
	DRAW_RANGE,	// [begin, end)
};

HexView::HexView(QWidget *parent, Document *doc, Cursor *cur, Highlight *hi)
	: ::View(parent, doc, hi)
	, cur_(cur)
{
}

void HexView::resizeEvent(QResizeEvent *rs)
{
	QSize size(min(rs->size().width(), config_.maxWidth()), rs->size().height());
	QResizeEvent resize(size, rs->oldSize());
	View::resizeEvent(&resize);
}

void HexView::refreshPixmap()
{
	refreshPixmap(DRAW_ALL);
}

void HexView::refreshPixmap(int type, int line, int end)
{
	Q_ASSERT(0 <= line && line < doc_->length() / HexConfig::Num + 1);
	Q_ASSERT(0 <= end && end < doc_->length() / HexConfig::Num + 1);
	qDebug("refresh event type:%d line:%d end:%d", type, line, end);
	// TODO: Optimizing drawing

	//pix_.fill(config_.Colors[Color::Background]);

	if (!doc_->length()) {
		// TODO: draw Empty Background only
		return;
	}

	QPainter painter(&pix_);
	painter.setFont(config_.Font);

	// Compute drawing area
	int yt = config_.top();
	int y = config_.top() + config_.byteMargin().top();
	int yCount, yMax;

	switch (type) {
	case DRAW_ALL:
		yCount = config_.drawableLines(height());
		break;
	case DRAW_LINE:
		yt += config_.byteHeight() * line;
		y  += config_.byteHeight() * line;
		yCount = 1;
		break;
	case DRAW_AFTER:
		yt += config_.byteHeight() * line;
		y  += config_.byteHeight() * line;
		yMax = max(y + config_.byteHeight(), height());
		yCount = config_.drawableLines(yMax - y);
		break;
	case DRAW_RANGE:
		yt += config_.byteHeight() * line;
		y  += config_.byteHeight() * line;
		yMax = min(y + config_.byteHeight() * end, height());
		yCount = config_.drawableLines(yMax - y);
		break;
	}
	quint64 top = (cur_->Top + line) * HexConfig::Num;
	const uint size = min(doc_->length() - top, (quint64)HexConfig::Num * yCount);

	// Compute selectead area
	const int xb = 0, xe = width();
	bool selected = false;
	quint64 sb = 0, se = 0;
	if (cur_->Selected) {
		sb = min(cur_->SelBegin, cur_->SelEnd);
		se = max(cur_->SelBegin, cur_->SelEnd);
		if (top <= se) {
			const quint64 vpos_end = max(top + (HexConfig::Num * yCount), top + size);
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

	// Draw
	drawLines(painter, y, yt);
	update(0, yt, min(width(), config_.maxWidth()), yCount * config_.byteHeight());
}

void HexView::drawLines(QPainter &painter, int y, int yt)
{
	// Draw lines
	DCIList::iterator itr = dcolors_.begin(), end = dcolors_.end();
	QBrush br;
	bool init_itr = false;
	QString hex;
	hex.resize(2);

	for (int i = 0, j = 0, count = 0; itr != end;) {
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
		count = min((int)(itr->Length), HexConfig::Num - j);
		qDebug("itr->Length:%d j:%d count:%d", itr->Length, j, count);
		// Draw background
		int width;
		int begin = config_.x(j) - config_.ByteMargin.left();
		if (2 <= count) {
			width = config_.X(j + count - 1) - begin;
		} else {
			width = config_.byteWidth();
		}
		painter.fillRect(begin, yt, width, config_.byteHeight(), br);

		// Draw text
		for (int k = 0; k < count; k++, i++, j++) {
			byteToHex(buff_[i], hex);
			painter.drawText(config_.x(j), y, config_.charWidth(2), config_.byteHeight(), Qt::AlignCenter, hex);
		}
		qDebug("y: %d, yt:%d", y, yt);

		itr->Length -= count;
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
}

void HexView::byteToHex(uchar c, QString &h)
{
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
		grabMouse();
		drawSelected(true);

		cur_->SelBegin = cur_->SelEnd = cur_->SelEndO = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->Toggle = true;
		qDebug("press -  begin:%lld", cur_->SelBegin);
	}
}

void HexView::mouseMoveEvent(QMouseEvent *ev)
{
	if (cur_->Toggle) {
		cur_->SelEndO = cur_->SelEnd;
		cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->refreshSelected();

		drawSelected();
	}
}

void HexView::mouseReleaseEvent(QMouseEvent *ev)
{
	if (cur_->Toggle) {
		releaseMouse();
		quint64 oldBegin = cur_->SelBegin;
		quint64 oldEnd = cur_->SelEnd;

		cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->refreshSelected();
		cur_->Toggle = false;
		qDebug("mouse release - begin:%lld end:%lld", cur_->SelBegin, cur_->SelEnd);

		drawSelected();
	}
}

#define MIN(a,b) ((a) < (b) ? (a) : (b))
quint64 HexView::moveByMouse(int xx, int yy)
{
	int x = config_.XToPos(xx);
	int y = config_.YToLine(yy);

	qDebug("move x:%d y:%d top: %lld", x, y, cur_->Top);

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		x = y = 0;
	}

	cur_->Position = MIN(cur_->Top + x + y * HexConfig::Num, doc_->length());
	qDebug("Position: %lld", cur_->Position);
	return cur_->Position;
}
#undef MIN

void HexView::drawSelected(bool reset)
{
	quint64 b, e;
	if (reset) {
		b = min(min(cur_->SelBegin, cur_->SelEnd), cur_->SelEndO);
		e = max(max(cur_->SelBegin, cur_->SelEnd), cur_->SelEndO);
		const int bL = b / HexConfig::Num - cur_->Top;
		const int eL = e / HexConfig::Num - cur_->Top + 1;
		cur_->Selected = false;
		qDebug("reset - bL:%d eL:%d", bL, eL);
		qDebug("reset - begin:%lld end:%lld endO:%lld", cur_->SelBegin, cur_->SelEnd, cur_->SelEndO);
		refreshPixmap(DRAW_RANGE, bL, eL);
	} else if (cur_->selMoved()) {
		if ((cur_->SelBegin < cur_->SelEndO && cur_->SelBegin >= cur_->SelEnd ||
			cur_->SelBegin >= cur_->SelEndO && cur_->SelBegin < cur_->SelEnd)) {
			// Crossing between begin and end
			b = min(min(cur_->SelBegin, cur_->SelEnd), cur_->SelEndO);
			e = max(max(cur_->SelBegin, cur_->SelEnd), cur_->SelEndO);
			qDebug("cross end:%lld endO:%lld", cur_->SelEnd, cur_->SelEndO);
		} else {
			// Minimum area
			b = min(cur_->SelEnd, cur_->SelEndO);
			e = max(cur_->SelEnd, cur_->SelEndO);
			qDebug("minimum end:%lld endO:%lld", cur_->SelEnd, cur_->SelEndO);
		}

		const int bL = b / HexConfig::Num - cur_->Top;
		const int eL = e / HexConfig::Num - cur_->Top + 1;
		qDebug("bL:%d eL:%d", bL, eL);
		refreshPixmap(DRAW_RANGE, bL, eL);
	}
}

}	// namespace
