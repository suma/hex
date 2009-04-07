
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
	, ByteMargin(3, 0, 2, 0)
	, Font("Monaco", 17)
	, EnableCaret(true)
	, CaretBlinkTime(500)
	, FontMetrics(Font)
{
	// Coloring
	Colors[Color::Background] = QColor(0xEF,0xEF,0xEF);
	Colors[Color::Text] = QColor(0,0,0);
	Colors[Color::SelBackground] = QColor(0xA0,0xA0,0xFF);
	Colors[Color::SelText] = QColor(0,0,0);
	Colors[Color::CaretBackground] = QColor(0xFF, 0, 0);
	Colors[Color::CaretText] = QColor(0xFF,0xFF,0xFF);

	// Font
	Font.setFixedPitch(true);

	update();
}

void HexConfig::update()
{
	// TODO: set ByteMargin value(left=charWidth/2, right=charWidth/2)

	// Pos
	x_begin[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_begin[i] = x_begin[i-1] + byteWidth();
	}

	// Pos of end
	for (int i = 0; i < Num; i++) {
		x_end[i] = x_begin[i] + charWidth(2) + ByteMargin.right();
	}

	// Area
	x_area[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_area[i] = x_area[i-1] + byteWidth();
	}
	x_area[Num] = x_area[Num-1] + byteWidth();
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

	return (int)distance(x_area, lower_bound(x_area, x_area + Num + 1, x)) - 1;
}

int HexConfig::YToLine(int y) const
{
	if (y < top()) {
		return -1;
	}
	return (y - top()) / byteHeight();
}

////////////////////////////////////////
// View

HexView::HexView(QWidget *parent, Document *doc, Highlight *hi)
	: ::View(parent, doc, hi)
	, cursor(new Cursor(doc, this))
{
	// Enable keyboard input
	setFocusPolicy(Qt::WheelFocus);
}

void HexView::resizeEvent(QResizeEvent *rs)
{
	QSize size(qMin(rs->size().width(), config.maxWidth()), rs->size().height());
	QResizeEvent resize(size, rs->oldSize());
	View::resizeEvent(&resize);
	pix_.fill(config.Colors[Color::Background]);
	drawView();
}

void HexView::drawView(DrawMode mode, int line_start, int end)
{
	//qDebug("refresh event mode:%d line:%d end:%d", mode, line_start, end);
	//qDebug(" pos:%llu, anchor:%llu", cursor->Position, cursor->PositionAnchor);

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
	int count_draw_line, max_y;

	// Get minumum drawing area
	switch (mode) {
	case DRAW_ALL:
		count_draw_line = config.drawableLines(height());
		break;
	case DRAW_LINE:
		y_top += config.byteHeight() * line_start;
		y     += config.byteHeight() * line_start;
		count_draw_line = 1;
		break;
	case DRAW_AFTER:
		y_top += config.byteHeight() * line_start;
		y     += config.byteHeight() * line_start;
		max_y = qMax(y + config.byteHeight(), height());
		count_draw_line = config.drawableLines(max_y - y);
		break;
	case DRAW_RANGE:
		y_top += config.byteHeight() * line_start;
		y     += config.byteHeight() * line_start;
		max_y = qMin(y + config.byteHeight() * end, height());
		count_draw_line = config.drawableLines(max_y - y);
		break;
	}

	// Get top position of view
	const quint64 top = (cursor->Top + line_start) * HexConfig::Num;
	const uint size = qMin(document->length() - top, (quint64)HexConfig::Num * count_draw_line);
	if (size == 0) {
		return;
	}

	// Draw empty area(after end line)
	if (mode == DRAW_ALL || mode == DRAW_AFTER) {
		//qDebug("draw empty area DRAW_ALL or DRAW_AFTER");
		QBrush brush(config.Colors[Color::Background]);
		const int y_start = y_top + qMax(0, count_draw_line - 1) * config.byteHeight();
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
	isSelected(selected, sel_begin, sel_end, top, count_draw_line, size);

	// TODO: Adding cache class for color highligh data
	::DrawInfo di(y, top, sel_begin, sel_end, size, selected);
	getDrawColors(di, dcolors_);

	// Draw lines
	//qDebug("x:%d", (width() - config.Margin.left()) / config.byteWidth());
	const int x_count_max = (width() - config.Margin.left()) / config.byteWidth() + 1;
	drawLines(painter, dcolors_, y_top, 0, x_count_max);

	// Update screen buffer
	const int draw_width  = qMin(width(), config.maxWidth());
	const int draw_height = count_draw_line * config.byteHeight();
	painter.end();
	update(0, y_top, draw_width, draw_height);
}

inline void HexView::isSelected(bool &selected, quint64 &sel_begin, quint64 &sel_end, quint64 top, int count_draw_line, uint size)
{
	if (!cursor->hasSelection()) {
		return;
	}

	sel_begin = qMin(cursor->Position, cursor->PositionAnchor);
	sel_end   = qMax(cursor->Position, cursor->PositionAnchor);

	if (top <= sel_end && sel_begin <= qMax(top + (HexConfig::Num * count_draw_line), top + size)) {
		selected = true;
	} else {
		selected = false;
	}
}

inline bool HexView::isSelected(quint64 pos)
{
	const quint64 sel_begin = qMin(cursor->Position, cursor->PositionAnchor);
	const quint64 sel_end   = qMax(cursor->Position, cursor->PositionAnchor);
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
		//qDebug("empty: %d", x);
		QBrush brush(config.Colors[Color::Background]);
		painter.fillRect(config.x(x), y, width(), config.byteHeight(), brush);
	}
}

inline void HexView::drawText(QPainter &painter, const QString &hex, int x, int y)
{
	painter.drawText(x, y, config.charWidth(2), config.charHeight(), Qt::AlignCenter, hex);
}

void HexView::drawCaret(bool visible)
{
	drawCaret(visible, cursor->Position);
}

void HexView::drawCaret(bool visible, quint64 pos)
{
	// Check out of range
	if (!(config.top() + config.byteHeight() < height())) {
		return;
	}

	// Redraw line
	const quint64 line = cursor->Position / HexConfig::Num;
	if (cursor->Top <= line && line - cursor->Top < (unsigned int)config.drawableLines(height())) {
		drawView(DRAW_LINE, line - cursor->Top);
	}

	// Shape
	const CaretShape shape = visible ? cursor->CaretVisibleShape : cursor->CaretInvisibleShape;
	if (shape == CARET_NONE) {
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
	drawCaretShape(CaretDrawInfo(painter, shape, pos, x, y, pos < document->length()));

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

	switch (info.shape) {
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
	if (cursor->HighNibble || !info.caret_middle) {
		x = config.x(info.x);
	} else {
		x = config.x(info.x) + config.ByteMargin.left() + config.charWidth();
	}
	QBrush brush(config.Colors[Color::CaretBackground]);
	info.painter.fillRect(x, info.y, 2, config.byteHeight(), brush);
}

void HexView::drawCaretBlock(const CaretDrawInfo &info)
{
	if (info.caret_middle) {
		if (cursor->HighNibble || cursor->hasSelection()) {
			// Draw block byte
			QBrush brush(config.Colors[Color::CaretBackground]);
			info.painter.setBackground(brush);
			info.painter.setPen(config.Colors[Color::CaretText]);
			info.painter.fillRect(config.x(info.x), info.y, config.byteWidth(), config.byteHeight(), brush);
			info.painter.drawText(config.x(info.x) + config.ByteMargin.left(), info.y + config.ByteMargin.top(), config.charWidth(2), config.charHeight(), Qt::AlignCenter, info.hex);
		} else {
			// Draw block lowwer nibble
			QBrush brush(config.Colors[Color::CaretBackground]);
			info.painter.setBackground(brush);
			info.painter.setPen(config.Colors[Color::CaretText]);
			info.painter.fillRect(config.x(info.x) + config.ByteMargin.left() + config.charWidth(), info.y, config.charWidth() + config.ByteMargin.right(), config.byteHeight(), brush);
			QString low(info.hex[1]);
			info.painter.drawText(config.x(info.x) + config.ByteMargin.left() + config.charWidth(), info.y + config.ByteMargin.top(), config.charWidth(2), config.charHeight(), Qt::AlignLeft, low);
		}
	} else {
		// Draw block without data
		QBrush brush(config.Colors[Color::CaretBackground]);
		info.painter.fillRect(config.x(info.x), info.y, config.byteWidth(), config.byteHeight(), brush);
	}
}

void HexView::drawCaretFrame(const CaretDrawInfo &info)
{
	int width, x;
	if (cursor->HighNibble || !info.caret_middle) {
		width = config.byteWidth() - 1;
		x = config.x(info.x);
	} else {
		width = config.charWidth() + config.ByteMargin.right() - 1;
		x = config.x(info.x) + config.charWidth() + config.ByteMargin.left();
	}
	info.painter.setPen(config.Colors[Color::CaretBackground]);
	info.painter.drawRect(x, info.y, width, config.byteHeight() - 1);
}

void HexView::drawCaretUnderbar(const CaretDrawInfo &info)
{
	int width, x;
	if (cursor->HighNibble || !info.caret_middle) {
		width = config.byteWidth() - 1;
		x = config.x(info.x);
	} else {
		width = config.charWidth() + config.ByteMargin.right() - 1;
		x = config.x(info.x) + config.ByteMargin.left() + config.charWidth();
	}

	QBrush brush(config.Colors[Color::CaretBackground]);
	info.painter.fillRect(x, info.y + config.byteHeight() - 2, width, 2, brush);
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

		cursor->movePosition(posAt(ev->pos()), false, false);

		// Start mouse capture
		grabMouse();
	}
}

void HexView::mouseMoveEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) {
		// FIXME: move down automatically
		if (height() < ev->pos().y()) {
			return;
		}
		cursor->movePosition(posAt(ev->pos()), true, false);
	}
}

void HexView::mouseReleaseEvent(QMouseEvent *)
{
	//qDebug("mouse release");

	// End mouse capture
	releaseMouse();
}

quint64 HexView::posAt(const QPoint &pos)
{
	int x = config.XToPos(pos.x());
	int y = config.YToLine(pos.y());

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		x = y = 0;
	}

	return qMin((cursor->Top + y) * HexConfig::Num + x, document->length());
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
		cursor->turnHexCaretVisible();
	}
}

void HexView::keyPressEvent(QKeyEvent *ev)
{
	if (ev == QKeySequence::SelectAll) {
		//ev->accept();
		//all
		return;
	} else if (ev == QKeySequence::Undo) {
		return;
	} else if (ev == QKeySequence::Redo) {
		return;
	}


	// TODO: support keyboard remap
	if (ev->modifiers() == Qt::NoModifier) {
		//qDebug("keypress:[%s]", ev->text().toStdString().c_str());
	} else {
		//qDebug("keypress:[%s] with modifier", ev->text().toStdString().c_str());
	}

	bool keepAnchor = ev->modifiers() & Qt::SHIFT ? true : false;
	switch (ev->key()) {
	case Qt::Key_Home:
		cursor->movePosition(0, keepAnchor, false);
		break;
	case Qt::Key_End:
		cursor->movePosition(document->length(), keepAnchor, false);
		break;
	case Qt::Key_Left:
		cursor->moveRelativePosition(-1, keepAnchor, false);
		break;
	case Qt::Key_Right:
		cursor->moveRelativePosition(1, keepAnchor, false);
		break;
	case Qt::Key_Up:
		cursor->moveRelativePosition(-16, keepAnchor, false);
		break;
	case Qt::Key_Down:
		cursor->moveRelativePosition(16, keepAnchor, false);
		break;
	case Qt::Key_PageUp:
		cursor->moveRelativePosition(-16 * 15, keepAnchor, true);
		break;
	case Qt::Key_PageDown:
		cursor->moveRelativePosition(16 * 15, keepAnchor, true);
		break;
	case Qt::Key_Backspace:
		qDebug("key backspace");
		break;
	case Qt::Key_Insert:
		qDebug("key insert");
		break;
	case Qt::Key_Delete:
		qDebug("key delete");
		break;
	default:
		{
			QString text = ev->text();
			for (int i = 0; i < text.length(); i++) {
				QChar c = text.at(i);
				int nibble = -1;
				if (c.unicode() >= 'a' && c.unicode() <= 'f') {
					nibble = c.unicode() - 'a' + 10;
				} else if (c.unicode() >= '0' && c.unicode() <= '9') {
					nibble = c.unicode() - '0';
				}
				if (nibble < 0) {
					continue;
				}
				if (cursor->HighNibble) {
					//changeData(m_cursorPosition, (nibble << 4) + (m_data[m_cursorPosition] & 0x0f), true);
					cursor->HighNibble = false;
				} else {
					//changeData(m_cursorPosition, nibble + (m_data[m_cursorPosition] & 0xf0));
					cursor->HighNibble = true;
					cursor->moveRelativePosition(1, false, false);
				}
			}
		}
		return;
	}

	if (ev->modifiers() != Qt::NoModifier) {
	} else {
	}
}

}	// namespace
