
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
	//, ByteMargin(1, 1, 1, 0)
	//, Font("Courier", 13)
	, Font("Monaco", 13)
	, EnableCaret(true)
	, CaretBlinkTime(500)
	, FontMetrics(Font)
{
	// Coloring
	Colors[Color::Background] = QColor(0xCE,0xFF,0xCE);
	Colors[Color::Text] = QColor(0,0,0);
	Colors[Color::SelBackground] = QColor(0xCC,0xCC,0xFF);
	Colors[Color::SelText] = QColor(0,0x40,0x40);
	HexCaretColor = QColor(0xFF, 0, 0, 128);

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
	pix_.fill(config_.Colors[Color::Background]);
	off_ = pix_;
	refreshPixmap();
}

void HexView::refreshPixmap()
{
	refreshPixmap(DRAW_ALL);
}

void HexView::refreshPixmap(int type, int line, int end)
{
	qDebug("refresh event type:%d line:%d end:%d", type, line, end);

	//QPainter painter(&pix_);
	QPainter painter(&off_);
	painter.setFont(config_.Font);

	if (!doc_->length()) {
		// TODO: draw Empty Background only
		QBrush brush(config_.Colors[Color::Background]);
		painter.fillRect(0, 0, width(), height(), brush);
		painter.end();
		pix_ = off_;
		update(0, 0, width(), height());
		return;
	}

	Q_ASSERT(0 <= line && line <= doc_->length() / HexConfig::Num + 1);
	Q_ASSERT(0 <= end && end <= doc_->length() / HexConfig::Num + 1);

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
		yMax = min(y + config_.byteHeight(), height());
		yCount = config_.drawableLines(yMax - y);
		break;
	case DRAW_RANGE:
		yt += config_.byteHeight() * line;
		y  += config_.byteHeight() * line;
		yMax = min(y + config_.byteHeight() * end, height());
		yCount = config_.drawableLines(yMax - y);
		break;
	}

	// Index of view top
	quint64 top = (cur_->Top + line) * HexConfig::Num;
	const uint size = min(doc_->length() - top, (quint64)HexConfig::Num * yCount);

	// Draw empty area(after end line)
	if (type == DRAW_ALL && yMax < height()) {
		qDebug("draw empty area yMax:%d height:%d", yMax, height());
		QBrush brush(config_.Colors[Color::Background]);
		QRect rc(0, yMax, width(), height());
		painter.fillRect(rc, brush);
	}

	// Copy from document
	qDebug("Document::get(%llu, .., %u)", top, size);
	if (buff_.capacity() < size) {
		buff_.reserve(size);
	}
	doc_->get(top, &buff_[0], size);

	// Compute selectead area
	bool selected = false;
	quint64 sb = 0, se = 0;
	isSelected(selected, sb, se, top, yCount, size);

	// TODO: Adding cache class for computed values if this function is bottle neck
	::DrawInfo di(y, top, sb, se, size, selected);
	getDrawColors(di, dcolors_, config_.Colors);

	// Draw Background clear
	QBrush brush(config_.Colors[Color::Background]);
	QRect rc(0, yt, width(), yt+config_.byteHeight());
	painter.fillRect(rc, brush);

	// Draw
	drawLines(painter, y, yt);
	painter.end();

	QPainter pixpainter(&pix_);
	int drawwidth = min(width(), config_.maxWidth());
	int drawheight= yCount * config_.byteHeight();
	QRect cprc(0, yt, drawwidth, drawheight);
	pixpainter.drawPixmap(cprc, off_, cprc);
	update(0, yt, drawwidth, drawheight);
}

inline void HexView::isSelected(bool &selected, quint64 &sel_begin, quint64 &sel_end, quint64 top, int yCount, uint size)
{
	if (cur_->Selected) {
		sel_begin = min(cur_->SelBegin, cur_->SelEnd);
		sel_end   = max(cur_->SelBegin, cur_->SelEnd);
		if (top <= sel_end) {
			const quint64 vpos_end = max(top + (HexConfig::Num * yCount), top + size);
			if (sel_begin <= vpos_end) {
				selected = true;
			}
		}
	}
}

void HexView::drawLines(QPainter &painter, int y, int yt)
{
	// Draw lines
	DCIList::iterator itr = dcolors_.begin(), end = dcolors_.end();
	QBrush brush;
	bool init_itr = false;
	QString hex;
	hex.resize(2);

	// Draw text each lines and colors
	for (int i = 0, j = 0, count = 0; itr != end;) {
		if (!init_itr) {
			// Create brush
			brush = QBrush(itr->Colors[Color::Background]);

			// Set color
			painter.setBackground(brush);
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
		painter.fillRect(begin, yt, width, config_.byteHeight(), brush);

		// Draw text
		for (int k = 0; k < count; k++, i++, j++) {
			byteToHex(buff_[i], hex);
			painter.drawText(config_.x(j), y, config_.charWidth(2), config_.charHeight(), Qt::AlignCenter, hex);
		}
		qDebug("y: %d, yt:%d", y, yt);

		itr->Length -= count;
		j = j % HexConfig::Num;
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

void HexView::drawCaret(bool visible)
{
	drawCaret(visible, cur_->Position, config_.top(), height());
	emit caretChanged(visible, cur_->Position);
}

void HexView::drawCaret(bool visible, quint64 pos)
{
	drawCaret(visible, pos, config_.top(), height());
	emit caretChanged(visible, pos);
}

void HexView::redrawCaret()
{
	drawCaret(false, cur_->SelEndO);
	drawCaret(true);
}

void HexView::drawCaret(bool visible, quint64 position, int ytop, int ymax)
{
	qDebug("drawCaret visible:%d ytop:%d ymax:%d sel:%llu top:%llu", visible, ytop, ymax, position, cur_->Top);

	int line = position / HexConfig::Num - cur_->Top;
	const int x = position % HexConfig::Num;
	const int yt = ytop + line * config_.byteHeight();
	qDebug("caret (line:%d x:%d)", line, x);

	if (!(ytop <= yt && ytop + config_.byteHeight() < ymax)) {
		return;
	}

	QPainter painter(&pix_);
	const int dx = config_.x(x);
	const int dw = config_.caretWidth();
	const int dh = config_.caretHeight();

	if (visible) {
		QBrush brush(config_.HexCaretColor);
		painter.fillRect(dx, yt, dw, dh, brush);
	} else {
		painter.drawPixmap(dx, yt, dw, dh, off_, dx, yt, dw, dh);
	}

	update(dx, yt, dw, dh);
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

		cur_->SelEndO = cur_->Position;
		cur_->SelBegin = cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->Toggle = true;

		if (config_.EnableCaret && cur_->SelEnd != cur_->SelEndO) {
			redrawCaret();
			cur_->HexCaretVisible = false;
		}
		qDebug("press -  begin:%lld", cur_->SelBegin);
	}
}

void HexView::mouseMoveEvent(QMouseEvent *ev)
{
	if (cur_->Toggle) {
		cur_->SelEndO = cur_->SelEnd;

		cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->refreshSelected();

		drawSelected(false);

		if (config_.EnableCaret && cur_->SelEnd != cur_->SelEndO) {
			drawCaret(false, cur_->SelEndO);
			drawCaret(true);
			redrawCaret();
			cur_->HexCaretVisible = false;
		}
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

		drawSelected(false);

		if (config_.EnableCaret && cur_->SelEnd != cur_->SelEndO) {
			redrawCaret();
			cur_->HexCaretVisible = false;
		}
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
	if (reset && cur_->Selected) {
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

void HexView::setCaretBlink(bool enable)
{
	if (!config_.EnableCaret || !config_.CaretBlinkTime) {
		return;
	}
	if (enable) {
		if (cur_->CaretTimerId == 0) {
			cur_->CaretTimerId = startTimer(config_.CaretBlinkTime);
		}
	} else {
		if (cur_->CaretTimerId != 0) {
			killTimer(cur_->CaretTimerId);
			cur_->CaretTimerId = 0;
		}
	}
}

void HexView::timerEvent(QTimerEvent *ev)
{
	if (cur_->CaretTimerId == ev->timerId()) {
		drawCaret(cur_->HexCaretVisible);
		cur_->HexCaretVisible = !cur_->HexCaretVisible;
	}
}


}	// namespace
