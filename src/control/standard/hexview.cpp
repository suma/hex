
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
	, ByteMargin(2, 0, 2, 0)
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
	Colors[Color::CaretText] = QColor(0xFA,0xFA,0x20);

	// Font
	Font.setFixedPitch(true);

	update();
}

void HexConfig::update()
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
	, cursor(new Cursor(doc, this))
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
	drawView();
}

void HexView::drawView()
{
	drawView(DRAW_ALL);
}

void HexView::drawView(int type, int line_start, int end)
{
	qDebug("refresh event type:%d line:%d end:%d", type, line_start, end);
	qDebug(" end:%llu endOld:%llu pos:%llu", cursor->SelEnd, cursor->SelEndOld, cursor->Position);

	// FIXME: refactoring refresh event
	QPainter painter;
	painter.begin(&pix_);
	painter.setFont(config.Font);

	if (!document->length()) {
		// TODO: draw Empty Background only
		QBrush brush(config.Colors[Color::Background]);
		painter.fillRect(0, 0, width(), height(), brush);
		painter.end();
		// Update screen buffer
		update(0, 0, width(), height());
		return;
	}

	Q_ASSERT(0 <= line_start && line_start <= document->length() / HexConfig::Num + 1);
	Q_ASSERT(0 <= end && end <= document->length() / HexConfig::Num + 1);

	// Get draw range
	int y_top = config.top();
	int y = config.top() + config.byteMargin().top();
	int count_line, max_y;

	// Get minumum drawing area
	switch (type) {
	case DRAW_ALL:
		count_line = config.drawableLines(height());
		break;
	case DRAW_LINE:
		y_top += config.byteHeight() * line_start;
		y     += config.byteHeight() * line_start;
		count_line = 1;
		break;
	case DRAW_AFTER:
		y_top += config.byteHeight() * line_start;
		y     += config.byteHeight() * line_start;
		max_y = max(y + config.byteHeight(), height());
		count_line = config.drawableLines(max_y - y);
		break;
	case DRAW_RANGE:
		y_top += config.byteHeight() * line_start;
		y     += config.byteHeight() * line_start;
		max_y = min(y + config.byteHeight() * end, height());
		count_line = config.drawableLines(max_y - y);
		break;
	}

	// Get top position of view
	const quint64 top = (cursor->Top + line_start) * HexConfig::Num;
	const uint size = min(document->length() - top, (quint64)HexConfig::Num * count_line);
	if (size == 0) {
		return;
	}

	// Draw empty area(after end line)
	if (type == DRAW_ALL || type == DRAW_AFTER) {
		QBrush brush(config.Colors[Color::Background]);
		const int y_start = y_top + max(0, count_line - 1) * config.byteHeight();
		painter.fillRect(0, y_start, width(), height(), brush);
	}

	// Copy from document
	if (buff_.capacity() < size) {
		buff_.resize(size);
	}
	document->get(top, &buff_[0], size);

	// Get selectead area
	bool selected = false;
	quint64 sel_begin = 0, sel_end = 0;
	isSelected(selected, sel_begin, sel_end, top, count_line, size);

	// TODO: Adding cache class for color highligh data
	::DrawInfo di(y, top, sel_begin, sel_end, size, selected);
	getDrawColors(di, dcolors_);

	// Draw lines
	qDebug("x:%d", (width() - config.Margin.left()) / config.byteWidth());
	const int x_count_max = (width() - config.Margin.left()) / config.byteWidth() + 1;
	drawLines(painter, dcolors_, y_top, 0, x_count_max);
	painter.end();

	// Update screen buffer
	const int draw_width  = min(width(), config.maxWidth());
	const int draw_height = count_line * config.byteHeight();
	update(0, y_top, draw_width, draw_height);
}

inline void HexView::isSelected(bool &selected, quint64 &sel_begin, quint64 &sel_end, quint64 top, int count_line, uint size)
{
	if (!cursor->Selected) {
		return;
	}

	sel_begin = min(cursor->SelBegin, cursor->SelEnd);
	sel_end   = max(cursor->SelBegin, cursor->SelEnd);

	if (top <= sel_end && sel_begin <= max(top + (HexConfig::Num * count_line), top + size)) {
		selected = true;
	} else {
		selected = false;
	}
}

inline bool HexView::isSelected(quint64 pos)
{
	const quint64 sel_begin = min(cursor->SelBegin, cursor->SelEnd);
	const quint64 sel_end   = max(cursor->SelBegin, cursor->SelEnd);
	return sel_begin <= pos && pos <  sel_end;
}

void HexView::drawLines(QPainter &painter, DCIList &dcolors, int y, int x_begin, int x_end)
{
	int index_data = 0, x = 0;
	bool reset_color = true;
	QBrush brush;
	QString hex;
	hex.resize(2);

	for (DCIList::iterator itr_color = dcolors.begin(); itr_color != dcolors.end(); ) {
		// Setup/Update color settings
		if (reset_color) {
			// Create brush for background
			brush = QBrush(config.Colors[itr_color->BackgroundColor]);
			// Set color
			painter.setBackground(brush);
			painter.setPen(config.Colors[itr_color->TextColor]);
			reset_color = false;
		}

		// Skip
		if (x < x_begin || x_end <= x) {
			goto COUNTUP;
		}

		// Draw background
		painter.fillRect(config.x(x), y, config.byteWidth(), config.byteHeight(), brush);

		// Draw text
		byteToHex(buff_[index_data], hex);
		drawText(painter, hex, config.x(x) + config.ByteMargin.left(), y + config.ByteMargin.top());

COUNTUP:// Count up
		index_data++;
		x = (x + 1) % HexConfig::Num;

		// Iterate color
		Q_ASSERT(0 <= itr_color->Length);
		if (--itr_color->Length <= 0) {
			// Move next color
			++itr_color;
			// Make color update
			reset_color = true;
		}

		// Move next line
		if (x == 0) {
			y += config.byteHeight();
		}
	}

	// Draw empty area(after end line)
	if (0 < x && x < x_end && x < HexConfig::Num) {
		qDebug("empty: %d", x);
		QBrush brush(config.Colors[Color::Background]);
		painter.fillRect(config.x(x), y, width(), config.byteHeight(), brush);
	}
}

inline void HexView::drawText(QPainter &painter, const QString &hex, int x, int y)
{
	painter.drawText(x, y, config.charWidth(2), config.charHeight(), Qt::AlignCenter, hex);
}

void HexView::drawCaret(quint64 pos, int height_max)
{
	// Check out of range
	if (!(config.top() + config.byteHeight() < height_max)) {
		return;
	}

	// Begin paint
	QPainter painter;
	painter.begin(&pix_);
	painter.setFont(config.Font);

	// Get caret coordinates 
	const int x = pos % HexConfig::Num;
	const int y = config.top() + config.byteHeight() * (pos / HexConfig::Num - cursor->Top);

	// Draw shape
	drawCaretShape(CaretDrawInfo(painter, pos, x, y, pos < document->length()));

	// Finish paint and update screen buffer
	painter.end();
	update(config.x(x), y, config.byteWidth(), config.charHeight());
}

void HexView::drawCaretShape(CaretDrawInfo info)
{
	if (info.caret_middle) {
		// Copy from document
		uchar data;
		document->get(info.pos, &data, 1);

		info.hex.resize(2);
		byteToHex(data, info.hex);
	}

	switch (cursor->CaretShape) {
	case CARET_LINE:
		drawCaretLine(info);
		break;
	case CARET_BLOCK:
		drawCaretBlock(info);
		break;
	case CARET_FRAME:
		drawCaretFrame(info);
		break;
	case CARET_UNDERBAR:
		drawCaretUnderbar(info);
		break;
	}
}

void HexView::drawCaretLine(const CaretDrawInfo &info)
{
	int x;
	if (cursor->CaretHigh || !info.caret_middle) {
		x = config.x(info.x);
	} else {
		x = config.x(info.x) + config.ByteMargin.left() + config.charWidth();
	}
	QBrush brush(config.Colors[Color::CaretBackground]);
	info.painter.fillRect(x, info.y, 2, config.byteHeight(), brush);
}

void HexView::drawCaretUnderbar(const CaretDrawInfo &info)
{
	int width, x;
	if (cursor->CaretHigh || !info.caret_middle) {
		width = config.byteWidth() - 1;
		x = config.x(info.x);
	} else {
		width = config.charWidth() + config.ByteMargin.right() - 1;
		x = config.x(info.x) + config.ByteMargin.left() + config.charWidth();
	}

	QBrush brush(config.Colors[Color::CaretBackground]);
	info.painter.fillRect(x, info.y + config.byteHeight() - 2, width, 2, brush);
}

void HexView::drawCaretFrame(const CaretDrawInfo &info)
{
	int width, x;
	if (cursor->CaretHigh || !info.caret_middle) {
		width = config.byteWidth() - 1;
		x = config.x(info.x);
	} else {
		width = config.charWidth() + config.ByteMargin.right() - 1;
		x = config.x(info.x) + config.charWidth() + config.ByteMargin.left();
	}
	info.painter.setPen(config.Colors[Color::CaretBackground]);
	info.painter.drawRect(x, info.y, width, config.byteHeight() - 1);
}

void HexView::drawCaretBlock(CaretDrawInfo &info)
{
	if (info.caret_middle) {
		if (cursor->CaretHigh) {
			// Draw higher
			QBrush brush(config.Colors[Color::CaretBackground]);
			info.painter.setBackground(brush);
			info.painter.setPen(config.Colors[Color::CaretText]);
			info.painter.fillRect(config.x(info.x), info.y, config.byteWidth(), config.byteHeight(), brush);
			info.painter.drawText(config.x(info.x) + config.ByteMargin.left(), info.y + config.ByteMargin.top(), config.charWidth(2), config.charHeight(), Qt::AlignCenter, info.hex);
		} else {
			// Draw lowwer
			QBrush brush(config.Colors[Color::CaretBackground]);
			info.painter.setBackground(brush);
			info.painter.setPen(config.Colors[Color::CaretText]);
			info.painter.fillRect(config.x(info.x) + config.ByteMargin.left() + config.charWidth(), info.y, config.charWidth() + config.ByteMargin.right(), config.byteHeight(), brush);
			info.hex.remove(0, 1);
			info.painter.drawText(config.x(info.x) + config.ByteMargin.left() + config.charWidth(), info.y + config.ByteMargin.top(), config.charWidth(2), config.charHeight(), Qt::AlignLeft, info.hex);
		}
	} else {
		// Draw caret without data
		QBrush brush(config.Colors[Color::CaretBackground]);
		info.painter.fillRect(config.x(info.x), info.y, config.byteWidth(), config.byteHeight(), brush);
	}
}

void HexView::drawCaret(bool visible)
{
	if (visible) {
		drawCaret(cursor->Position, height());
		emit caretChanged(visible, cursor->Position);
	} else {
		drawCaret(false, cursor->Position);
	}
}

void HexView::drawCaret(bool visible, quint64 pos)
{
	if (visible) {
		// Draw
		drawCaret(pos, height());
	} else {
		// Set caret invisible
		quint64 line = cursor->Position / HexConfig::Num;
		if (cursor->Top <= line && line - cursor->Top < config.drawableLines(height())) {
			drawView(DRAW_LINE, line - cursor->Top);
		}
	}

	// Send carete refresh event
	emit caretChanged(visible, pos);
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
		qDebug("mosue press pos:%llu end:%llu endO:%llu el:%llu", cursor->Position, cursor->SelEnd, cursor->SelEndOld, cursor->SelEnd / HexConfig::Num);
		// Draw selected lines
		drawSelected(true);

		// Set begin position
		cursor->SelEndOld = cursor->Position;
		cursor->SelBegin = cursor->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());

		// Set caret visible
		cursor->Toggle = true;

		// -- Redraw lines if caret moved
		if (config.EnableCaret && cursor->SelEnd != cursor->SelEndOld) {
			const int pos = (cursor->SelEndOld / HexConfig::Num) - cursor->Top;
			if (pos <= config.drawableLines(height())) {
				drawView(DRAW_RANGE, pos, pos + 1);
			}
		}

		drawCaret();

		// Start mouse capture
		grabMouse();
	}
}

void HexView::mouseMoveEvent(QMouseEvent *ev)
{
	// Check mouse captured
	if (!cursor->Toggle) {
		return;
	}

	qDebug("mouse move");

	// Set moved position to OLD
	cursor->SelEndOld = cursor->SelEnd;

	// FIXME: move down automatically
	if (height() < ev->pos().y()) {
		return;
	}

	// Set moved position
	cursor->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());

	// Refresh flag
	cursor->refreshSelected();

	// Redraw updated lines
	drawSelected(false);

	// -- Redraw lines if caret moved
	if (config.EnableCaret && cursor->SelEnd != cursor->SelEndOld) {
		drawCaret();
		cursor->HexCaretVisible = false;
	}
}

void HexView::mouseReleaseEvent(QMouseEvent *ev)
{
	// Check mouse captured
	if (!cursor->Toggle) {
		return;
	}

	qDebug("mouse release");

	// End mouse capture
	releaseMouse();

	// Set moved position
	cursor->SelEnd = moveByMouse(ev->pos().x(), ev->pos().y());
	cursor->refreshSelected();

	// Set caret invisible
	cursor->Toggle = false;

	// Redraw updated lines
	drawSelected(false);

	// -- Redraw lines if caret moved
	if (config.EnableCaret && cursor->SelEnd != cursor->SelEndOld) {
		drawCaret();
		cursor->HexCaretVisible = false;
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

	cursor->Position = MIN((cursor->Top + y) * HexConfig::Num + x, document->length());
	return cursor->Position;
}
#undef MIN

void HexView::drawSelected(bool reset)
{
	quint64 begin, end;
	if (reset && cursor->Selected) {
		//--- Reset selected lines
		// Get selected lines
		begin = min(min(cursor->SelBegin, cursor->SelEnd), cursor->SelEndOld);
		end   = max(max(cursor->SelBegin, cursor->SelEnd), cursor->SelEndOld);
		const int begin_line = begin / HexConfig::Num - cursor->Top;
		const int end_line   = end   / HexConfig::Num - cursor->Top + 1;
		cursor->Selected = false;

		// Redraw lines
		drawView(DRAW_RANGE, begin_line, end_line);
	} else if (cursor->selMoved()) {
		// Selected range is changing
		if ((cursor->SelBegin < cursor->SelEndOld && cursor->SelBegin >= cursor->SelEnd ||
			cursor->SelBegin >= cursor->SelEndOld && cursor->SelBegin < cursor->SelEnd)) {
			// Crossing between begin and end
			begin = min(min(cursor->SelBegin, cursor->SelEnd), cursor->SelEndOld);
			end   = max(max(cursor->SelBegin, cursor->SelEnd), cursor->SelEndOld);
		} else {
			// Minimum range
			begin = min(cursor->SelEnd, cursor->SelEndOld);
			end   = max(cursor->SelEnd, cursor->SelEndOld);
		}

		// Get redrawing lines
		const int begin_line = begin / HexConfig::Num - cursor->Top;
		const int end_line   = end   / HexConfig::Num - cursor->Top + 1;
		// Redraw lines
		drawView(DRAW_RANGE, begin_line, end_line);
	}
}

// Enable caret blink
void HexView::setCaretBlink(bool enable)
{
	if (!config.EnableCaret || !config.CaretBlinkTime) {
		return;
	}
	if (enable) {
		if (cursor->CaretTimerId == 0) {
			cursor->CaretTimerId = startTimer(config.CaretBlinkTime);
		}
	} else {
		if (cursor->CaretTimerId != 0) {
			killTimer(cursor->CaretTimerId);
			cursor->CaretTimerId = 0;
		}
	}
}

void HexView::timerEvent(QTimerEvent *ev)
{
	if (cursor->CaretTimerId == ev->timerId()) {
		// Caret blink
		drawCaret(cursor->HexCaretVisible);
		cursor->HexCaretVisible = !cursor->HexCaretVisible;
	}
}

void HexView::keyPressEvent(QKeyEvent *ev)
{
	// TODO: support keyboard remap

	quint64 old = cursor->SelEnd;
	quint64 oldT = cursor->Top;
	switch (ev->key()) {
	case Qt::Key_Home:
		cursor->Home();
		break;
	case Qt::Key_End:
		cursor->End();
		break;
	case Qt::Key_Left:
		cursor->Left();
		break;
	case Qt::Key_Right:
		cursor->Right();
		break;
	case Qt::Key_Up:
		cursor->Up();
		break;
	case Qt::Key_Down:
		cursor->Down();
		break;
	case Qt::Key_PageUp:
		cursor->PageUp();
		break;
	case Qt::Key_PageDown:
		cursor->PageDown();
		break;
	default:
		return;
	}
	cursor->SelEndOld = cursor->SelEnd;

	if (ev->modifiers() != Qt::SHIFT) {
		cursor->resetSelection();
	}

	// FIXME: refactoring refresh event
	if (cursor->SelEnd != old || cursor->Top != oldT) {
		drawView();
		drawCaret();
	}

	if (ev->modifiers() != Qt::NoModifier) {
	} else {
	}
}

HexView::CaretDrawInfo::CaretDrawInfo(QPainter &p, quint64 pos, int x, int y, bool caret_middle)
	: painter(p)
{
	this->pos = pos;
	this->x = x;
	this->y = y;
	this->caret_middle = caret_middle;
}

}	// namespace
