
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
	: Margin(2, 2, 3, 3)
	, ByteMargin(2, 0, 2, 1)
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
	Colors[Color::CaretBackground] = QColor(0xFF, 0, 0);
	Colors[Color::CaretText] = QColor(0xAA,0xAA,0x20);

	// Font
	Font.setFixedPitch(true);

	calculate();
}

void HexConfig::calculate()
{
	// Pos
	x_[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_[i] = x_[i-1] + byteWidth();
	}

	// Pos of end
	for (int i = 0; i < Num; i++) {
		X_[i] = x_[i] + charWidth(2) + ByteMargin.right();
	}

	// Area
	xarea_[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		xarea_[i] = xarea_[i-1] + byteWidth();
	}
	xarea_[Num] = xarea_[Num-1] + byteWidth();

	top_ = Margin.top();
}

int HexConfig::drawableLines(int height) const
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

enum {
	DRAW_ALL = 0,
	DRAW_LINE,
	DRAW_AFTER,
	DRAW_RANGE,	// [begin, end)
};

HexView::HexView(QWidget *parent, Document *doc, Highlight *hi)
	: ::View(parent, doc, hi)
	, cur_(new Cursor(doc, this))
{
	// Enable keyboard input
	setFocusPolicy(Qt::WheelFocus);
}

void HexView::resizeEvent(QResizeEvent *rs)
{
	QSize size(min(rs->size().width(), config.maxWidth()), rs->size().height());
	QResizeEvent resize(size, rs->oldSize());
	View::resizeEvent(&resize);
	pix_.fill(config.Colors[Color::Background]);
	refreshPixmap();
}

void HexView::refreshPixmap()
{
	refreshPixmap(DRAW_ALL);
}

void HexView::refreshPixmap(int type, int line, int end)
{
	qDebug("refresh event type:%d line:%d end:%d", type, line, end);

	QPainter painter(&pix_);
	painter.setFont(config.Font);

	if (!doc_->length()) {
		// TODO: draw Empty Background only
		QBrush brush(config.Colors[Color::Background]);
		painter.fillRect(0, 0, width(), height(), brush);
		painter.end();
		update(0, 0, width(), height());
		return;
	}

	Q_ASSERT(0 <= line && line <= doc_->length() / HexConfig::Num + 1);
	Q_ASSERT(0 <= end && end <= doc_->length() / HexConfig::Num + 1);

	// Compute drawing area
	int y_top = config.top();
	int y = config.top() + config.byteMargin().top();
	int count_line, max_y;

	switch (type) {
	case DRAW_ALL:
		count_line = config.drawableLines(height());
		break;
	case DRAW_LINE:
		y_top += config.byteHeight() * line;
		y += config.byteHeight() * line;
		count_line = 1;
		break;
	case DRAW_AFTER:
		y_top += config.byteHeight() * line;
		y += config.byteHeight() * line;
		max_y = max(y + config.byteHeight(), height());
		count_line = config.drawableLines(max_y - y);
		break;
	case DRAW_RANGE:
		y_top += config.byteHeight() * line;
		y += config.byteHeight() * line;
		max_y = min(y + config.byteHeight() * end, height());
		count_line = config.drawableLines(max_y - y);
		break;
	}

	// Index of view top
	quint64 top = (cur_->Top + line) * HexConfig::Num;
	const uint size = min(doc_->length() - top, (quint64)HexConfig::Num * count_line);

	// Draw empty area(after end line)
	if (type == DRAW_ALL || type == DRAW_AFTER) {
		QBrush brush(config.Colors[Color::Background]);
		QRect rect(0, y_top, width(), height());
		painter.fillRect(rect, brush);
	}

	// Copy from document
	if (buff_.capacity() < size) {
		buff_.resize(size);
	}
	doc_->get(top, &buff_[0], size);

	// Compute selectead area
	bool selected = false;
	quint64 sel_begin = 0, sel_end = 0;
	isSelected(selected, sel_begin, sel_end, top, count_line, size);

	// TODO: Adding cache class for computed values if this function is bottle neck
	::DrawInfo di(y, top, sel_begin, sel_end, size, selected);
	getDrawColors(di, dcolors_);

	// Draw Background clear
	QBrush brush(config.Colors[Color::Background]);
	QRect rect_bg(0, y_top, width(), y_top + config.byteHeight());
	painter.fillRect(rect_bg, brush);

	// Draw
	drawLines(painter, dcolors_, y_top);
	painter.end();

	// Update real window
	QPainter painter_pix(&pix_);
	const int draw_width  = min(width(), config.maxWidth());
	const int draw_height = count_line * config.byteHeight();

	update(0, y_top, draw_width, draw_height);
}

inline void HexView::isSelected(bool &selected, quint64 &sel_begin, quint64 &sel_end, quint64 top, int count_line, uint size)
{
	if (!cur_->Selected) {
		return;
	}

	sel_begin = min(cur_->SelBegin, cur_->SelEnd);
	sel_end   = max(cur_->SelBegin, cur_->SelEnd);

	if (top <= sel_end) {
		selected = sel_begin <= max(top + (HexConfig::Num * count_line), top + size);
	}
}

inline bool HexView::isSelected(quint64 pos)
{
	quint64 sel_begin = min(cur_->SelBegin, cur_->SelEnd);
	quint64 sel_end   = max(cur_->SelBegin, cur_->SelEnd);
	return sel_begin <= pos && pos <  sel_end;
}

void HexView::drawLines(QPainter &painter, DCIList &dcolors, int y, int x_begin, int x_end)
{
	// Draw lines
	int index_byte = 0, x = 0;
	bool change_color = true;
	QBrush brush;
	QString hex;
	hex.resize(2);

	for (DCIList::iterator itr_color = dcolors.begin(); itr_color != dcolors.end(); ) {
		// Setup color settings
		if (change_color) {
			// Create brush for background
			brush = QBrush(config.Colors[itr_color->BackgroundColor]);
			// Set color
			painter.setBackground(brush);
			painter.setPen(config.Colors[itr_color->TextColor]);
			change_color = false;
		}
		if (x < x_begin || x_end <= x) {
			goto COUNTUP;
		}

		// Draw background
		painter.fillRect(config.x(x), y, config.byteWidth(), config.byteHeight(), brush);

		// Draw text
		byteToHex(buff_[index_byte], hex);
		drawText(painter, hex, config.x(x) + config.ByteMargin.left(), y + config.ByteMargin.top());

COUNTUP:// Count up
		index_byte++;
		x = (x + 1) % HexConfig::Num;

		// Iterate color
		if (--itr_color->Length <= 0) {
			// Move next color
			++itr_color;
			// Enable color change
			change_color = true;
		}

		// Move next line
		if (x == 0) {
			y += config.byteHeight();
		}
	}
}

inline void HexView::drawText(QPainter &painter, const QString &hex, int x, int y)
{
	painter.drawText(x, y, config.charWidth(2), config.charHeight(), Qt::AlignCenter, hex);
}

void HexView::drawCaret(bool visible, quint64 position, int height_max)
{
	qDebug("drawCaret visible:%d height_max:%d sel:%llu top:%llu", visible, height_max, position, cur_->Top);

	if (!(config.top() + config.byteHeight() < height_max)) {
		return;
	}

	QPainter painter(&pix_);
	painter.setFont(config.Font);
	const int line = position / HexConfig::Num - cur_->Top;
	const int x = position % HexConfig::Num;
	const int y = config.top() + config.byteHeight() * line;

	int caret_color = Color::CaretBackground;
	if (doc_->length() <= position) {
		// Draw background
		QBrush brush(config.Colors[Color::CaretBackground]);
		painter.fillRect(config.x(x), y, config.byteWidth(), config.byteHeight(), brush);
	} else {
		// Copy from document
		uchar data;
		doc_->get(position, &data, 1);

		QString hex;
		hex.resize(2);
		byteToHex(data, hex);

		// Draw
		QBrush brush(config.Colors[Color::CaretBackground]);
		painter.setBackground(brush);
		painter.setPen(config.Colors[Color::CaretText]);
		painter.fillRect(config.x(x), y, config.byteWidth(), config.byteHeight(), brush);

		painter.drawText(config.x(x) + config.ByteMargin.left(), y + config.ByteMargin.top(), config.charWidth(2), config.charHeight(), Qt::AlignCenter, hex);
	}

	painter.end();
	update(config.x(x), y, config.byteWidth(), config.charHeight());
}

void HexView::drawCaret(bool visible)
{
	drawCaret(visible, cur_->Position, height());
	emit caretChanged(visible, cur_->Position);
}

void HexView::drawCaret(bool visible, quint64 pos)
{
	drawCaret(visible, pos, height());
	emit caretChanged(visible, pos);
}

void HexView::redrawCaret()
{
	//drawCaret(false, cur_->SelEndOld);
	drawCaret(true);
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

		cur_->SelEndOld = cur_->Position;
		cur_->SelBegin = cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->Toggle = true;

		if (config.EnableCaret && cur_->SelEnd != cur_->SelEndOld) {
			redrawCaret();
			cur_->HexCaretVisible = false;
		}
	}
}

void HexView::mouseMoveEvent(QMouseEvent *ev)
{
	if (cur_->Toggle) {
		cur_->SelEndOld = cur_->SelEnd;

		cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->refreshSelected();

		drawSelected(false);

		if (config.EnableCaret && cur_->SelEnd != cur_->SelEndOld) {
			//drawCaret(false, cur_->SelEndOld);
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

		cur_->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
		cur_->refreshSelected();
		cur_->Toggle = false;

		drawSelected(false);

		if (config.EnableCaret && cur_->SelEnd != cur_->SelEndOld) {
			redrawCaret();
			cur_->HexCaretVisible = false;
		}
	}
}

#define MIN(a,b) ((a) < (b) ? (a) : (b))
quint64 HexView::moveByMouse(int xx, int yy)
{
	int x = config.XToPos(xx);
	int y = config.YToLine(yy);

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		x = y = 0;
	}

	cur_->Position = MIN(cur_->Top + x + y * HexConfig::Num, doc_->length());
	return cur_->Position;
}
#undef MIN

void HexView::drawSelected(bool reset)
{
	quint64 b, e;
	if (reset && cur_->Selected) {
		b = min(min(cur_->SelBegin, cur_->SelEnd), cur_->SelEndOld);
		e = max(max(cur_->SelBegin, cur_->SelEnd), cur_->SelEndOld);
		const int bL = b / HexConfig::Num - cur_->Top;
		const int eL = e / HexConfig::Num - cur_->Top + 1;
		cur_->Selected = false;
		refreshPixmap(DRAW_RANGE, bL, eL);
	} else if (cur_->selMoved()) {
		if ((cur_->SelBegin < cur_->SelEndOld && cur_->SelBegin >= cur_->SelEnd ||
			cur_->SelBegin >= cur_->SelEndOld && cur_->SelBegin < cur_->SelEnd)) {
			// Crossing between begin and end
			b = min(min(cur_->SelBegin, cur_->SelEnd), cur_->SelEndOld);
			e = max(max(cur_->SelBegin, cur_->SelEnd), cur_->SelEndOld);
		} else {
			// Minimum area
			b = min(cur_->SelEnd, cur_->SelEndOld);
			e = max(cur_->SelEnd, cur_->SelEndOld);
		}

		const int bL = b / HexConfig::Num - cur_->Top;
		const int eL = e / HexConfig::Num - cur_->Top + 1;
		refreshPixmap(DRAW_RANGE, bL, eL);
	}
}

void HexView::setCaretBlink(bool enable)
{
	if (!config.EnableCaret || !config.CaretBlinkTime) {
		return;
	}
	if (enable) {
		if (cur_->CaretTimerId == 0) {
			//cur_->CaretTimerId = startTimer(config.CaretBlinkTime);
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

void HexView::keyPressEvent(QKeyEvent *ev)
{
	// TODO: support keyboard remap
	switch (ev->key()) {
	case Qt::Key_Home:
		cur_->Home();
		break;
	case Qt::Key_End:
		cur_->End();
		break;
	case Qt::Key_Left:
		cur_->Left();
		break;
	case Qt::Key_Right:
		cur_->Right();
		break;
	case Qt::Key_Up:
		cur_->Up();
		break;
	case Qt::Key_Down:
		cur_->Down();
		break;
	case Qt::Key_PageUp:
		cur_->PageUp();
		break;
	case Qt::Key_PageDown:
		cur_->PageDown();
		break;
	default:
		return;
	}

	if (ev->modifiers() != Qt::SHIFT) {
		cur_->resetSelection();
	}
	// TODO: optimization: compute refresh area and
	// support keyboard macros(like Vim repeat command)
	refreshPixmap();
	redrawCaret();

	if (ev->modifiers() != Qt::NoModifier) {
	} else {
	}
}


}	// namespace
