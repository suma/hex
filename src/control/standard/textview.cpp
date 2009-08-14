
#include <QtGui>
#include <algorithm>
#include <vector>
#include "textview.h"
#include "textcursor.h"
#include "../document.h"
#include "../highlight.h"

using namespace std;

namespace Standard {

////////////////////////////////////////
// Config
TextConfig::TextConfig()
	: Margin(2, 2, 3, 3)
	, ByteMargin(0, 0, 0, 0)
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

void TextConfig::update()
{
	// TODO: set ByteMargin value(left=charWidth/2, right=charWidth/2)

	// Pos
	x_begin[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_begin[i] = x_begin[i-1] + byteWidth();
	}

	// Pos of end
	for (int i = 0; i < Num; i++) {
		x_end[i] = x_begin[i] + charWidth(1) + ByteMargin.right();
	}

	// Area
	x_area[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_area[i] = x_area[i-1] + byteWidth();
	}
	x_area[Num] = x_area[Num-1] + byteWidth();
}

int TextConfig::drawableLines(int height) const
{
	const int y = top() + byteMargin().top();
	return (height - y + byteHeight()) / byteHeight();
}

int TextConfig::XToPos(int x) const
{
	if (x < Margin.left()) {
		return -1;
	}

	return (int)distance(x_area, lower_bound(x_area, x_area + Num + 1, x)) - 1;
}

int TextConfig::YToLine(int y) const
{
	if (y < top()) {
		return -1;
	}
	return (y - top()) / byteHeight();
}

////////////////////////////////////////
// View

TextView::TextView(QWidget *parent, Document *doc, Highlight *hi)
	: ::View(parent, doc, hi)
	, cursor_(new TextCursor(doc, this))
{
	// Enable keyboard input
	setFocusPolicy(Qt::WheelFocus);
}

void TextView::resizeEvent(QResizeEvent *rs)
{
	QSize size(qMin(rs->size().width(), config_.maxWidth()), rs->size().height());
	QResizeEvent resize(size, rs->oldSize());
	View::resizeEvent(&resize);
	pix_.fill(config_.Colors[Color::Background]);
	drawView();
}

void TextView::drawView(DrawMode mode, int line_start, int end)
{
	//qDebug("refresh event mode:%d line:%d end:%d", mode, line_start, end);
	//qDebug(" pos:%llu, anchor:%llu", cursor_->Position, cursor_->PositionAnchor);

	// FIXME: refactoring refresh event
	QPainter painter;
	painter.begin(&pix_);
	painter.setFont(config_.Font);

	if (!document_->length()) {
		// TODO: draw Empty Background only
		QBrush brush(config_.Colors[Color::Background]);
		painter.fillRect(0, 0, width(), height(), brush);
		painter.end();
		// Update screen buffer
		update(0, 0, width(), height());
		return;
	}

	Q_ASSERT(0 <= line_start);
	Q_ASSERT(static_cast<uint>(line_start) <= document_->length() / TextConfig::Num + 1);
	Q_ASSERT(0 <= end);
	Q_ASSERT(static_cast<uint>(end) <= document_->length() / TextConfig::Num + 1);

	// Get draw range
	int y_top = config_.top();
	int y = config_.top() + config_.byteMargin().top();
	int count_draw_line, max_y;

	// Get minumum drawing area
	switch (mode) {
	case DRAW_ALL:
		count_draw_line = config_.drawableLines(height());
		break;
	case DRAW_LINE:
		y_top += config_.byteHeight() * line_start;
		y     += config_.byteHeight() * line_start;
		count_draw_line = 1;
		break;
	case DRAW_AFTER:
		y_top += config_.byteHeight() * line_start;
		y     += config_.byteHeight() * line_start;
		max_y = qMax(y + config_.byteHeight(), height());
		count_draw_line = config_.drawableLines(max_y - y);
		break;
	case DRAW_RANGE:
		y_top += config_.byteHeight() * line_start;
		y     += config_.byteHeight() * line_start;
		max_y = qMin(y + config_.byteHeight() * end, height());
		count_draw_line = config_.drawableLines(max_y - y);
		break;
	}

	// Get top position of view
	const quint64 top = (cursor_->Top + line_start) * TextConfig::Num;
	const uint size = qMin(document_->length() - top, (quint64)TextConfig::Num * count_draw_line);
	if (size == 0) {
		return;
	}

	// Draw empty area(after end line)
	if (mode == DRAW_ALL || mode == DRAW_AFTER) {
		//qDebug("draw empty area DRAW_ALL or DRAW_AFTER");
		QBrush brush(config_.Colors[Color::Background]);
		const int y_start = y_top + qMax(0, count_draw_line - 1) * config_.byteHeight();
		painter.fillRect(0, y_start, width(), height(), brush);
	}

	// Copy from document
	if (buff_.capacity() < size) {
		buff_.resize(size);
	}
	document_->get(top, &buff_[0], size);

	// Get selectead area
	bool selected = false;
	quint64 sel_begin = 0, sel_end = 0;
	isSelected(selected, sel_begin, sel_end, top, count_draw_line, size);

	// TODO: Adding cache class for color highligh data
	::DrawInfo di(y, top, sel_begin, sel_end, size, selected);
	getDrawColors(di, dcolors_);

	// Draw lines
	//qDebug("x:%d", (width() - config_.Margin.left()) / config_.byteWidth());
	const int x_count_max = (width() - config_.Margin.left()) / config_.byteWidth() + 1;
	drawLines(painter, dcolors_, y_top, 0, x_count_max);

	// Update screen buffer
	const int draw_width  = qMin(width(), config_.maxWidth());
	const int draw_height = count_draw_line * config_.byteHeight();
	painter.end();
	update(0, y_top, draw_width, draw_height);
}

inline void TextView::drawViewAfter(quint64 pos)
{
	drawView(DRAW_AFTER, pos / TextConfig::Num - cursor_-> Top);
}

inline void TextView::isSelected(bool &selected, quint64 &sel_begin, quint64 &sel_end, quint64 top, int count_draw_line, uint size)
{
	if (!cursor_->hasSelection()) {
		return;
	}

	sel_begin = qMin(cursor_->Position, cursor_->PositionAnchor);
	sel_end   = qMax(cursor_->Position, cursor_->PositionAnchor);

	if (top <= sel_end && sel_begin <= qMax(top + (TextConfig::Num * count_draw_line), top + size)) {
		selected = true;
	} else {
		selected = false;
	}
}

inline bool TextView::isSelected(quint64 pos)
{
	const quint64 sel_begin = qMin(cursor_->Position, cursor_->PositionAnchor);
	const quint64 sel_end   = qMax(cursor_->Position, cursor_->PositionAnchor);
	return sel_begin <= pos && pos <  sel_end;
}

void TextView::drawLines(QPainter &painter, DCIList &dcolors, int y, int x_begin, int x_end)
{
	int index_data = 0, x = 0;
	bool reset_color = true;
	QBrush brush;
	QString str;
	str.resize(4);

	for (DCIList::iterator itr_color = dcolors.begin(); itr_color != dcolors.end(); ) {
		// Setup/Update color settings
		if (reset_color) {
			// Create brush for background
			brush = QBrush(config_.Colors[itr_color->BackgroundColor]);
			// Set color
			painter.setBackground(brush);
			painter.setPen(config_.Colors[itr_color->TextColor]);
			reset_color = false;
		}

		// Draw background
		painter.fillRect(config_.x(x), y, config_.byteWidth(), config_.byteHeight(), brush);

		// Draw text
		//byteToHex(buff_[index_data], hex);
		//int len = getLetterLength(buff[index]);

		int len = 2;
		//getLetter(str);
		//drawText(painter, str, config_.x(x) + config_.ByteMargin.left(), y + config_.ByteMargin.top());

		//index_data++;
		index_data += len;
		x = (x + 1) % TextConfig::Num;

		// Iterate color
		Q_ASSERT(0 <= itr_color->Length);
		if (--itr_color->Length <= 0) {
			// Move next color
			++itr_color;
			// Change color
			reset_color = true;
		}

		// Move next line
		if (x == 0) {
			y += config_.byteHeight();
		}
	}

	// Draw empty area(after end line)
	if (0 < x && x < x_end && x < TextConfig::Num) {
		//qDebug("empty: %d", x);
		QBrush brush(config_.Colors[Color::Background]);
		painter.fillRect(config_.x(x), y, width(), config_.byteHeight(), brush);
	}
}

inline void TextView::drawText(QPainter &painter, const QString &hex, int x, int y)
{
	painter.drawText(x, y, config_.charWidth(2), config_.charHeight(), Qt::AlignCenter, hex);
}

void TextView::drawCaret(bool visible)
{
	drawCaret(visible, cursor_->Position);
}

void TextView::drawCaret(bool visible, quint64 pos)
{
	// Check out of range
	if (!(config_.top() + config_.byteHeight() < height())) {
		return;
	}

	// Redraw line
	const quint64 line = cursor_->Position / TextConfig::Num;
	if (cursor_->Top <= line && line - cursor_->Top < static_cast<unsigned int>(config_.drawableLines(height()))) {
		drawView(DRAW_LINE, line - cursor_->Top);
	}

	// Shape
	const CaretShape shape = visible ? cursor_->CaretVisibleShape : cursor_->CaretInvisibleShape;
	if (shape == CARET_NONE) {
		return;
	}

	// Begin paint
	QPainter painter;
	painter.begin(&pix_);
	painter.setFont(config_.Font);

	// Get caret coordinates
	const int x = pos % TextConfig::Num;
	const int y = config_.top() + config_.byteHeight() * (pos / TextConfig::Num - cursor_->Top);

	// Draw shape
	drawCaretShape(CaretDrawInfo(painter, shape, pos, x, y, pos < document_->length()));

	// Finish paint and update screen buffer
	painter.end();
	update(config_.x(x), y, config_.byteWidth(), config_.charHeight());
}

void TextView::drawCaretShape(CaretDrawInfo info)
{
	if (info.caret_middle) {
		// Copy from document
		uchar data;
		document_->get(info.pos, &data, 1);

		info.hex.resize(2);
		//byteToHex(data, info.hex);
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
	default:
		;
	}
}

void TextView::drawCaretLine(const CaretDrawInfo &info)
{
	int x;
	//if (!info.caret_middle) {
	if (true) {
		x = config_.x(info.x);
	} else {
		x = config_.x(info.x) + config_.ByteMargin.left() + config_.charWidth();
	}
	QBrush brush(config_.Colors[Color::CaretBackground]);
	info.painter.fillRect(x, info.y, 2, config_.byteHeight(), brush);
}

void TextView::drawCaretBlock(const CaretDrawInfo &info)
{
	if (info.caret_middle) {
		//if (cursor_->hasSelection()) {
		if (true) {
			// Draw block byte
			QBrush brush(config_.Colors[Color::CaretBackground]);
			info.painter.setBackground(brush);
			info.painter.setPen(config_.Colors[Color::CaretText]);
			info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
			info.painter.drawText(config_.x(info.x) + config_.ByteMargin.left(), info.y + config_.ByteMargin.top(), config_.charWidth(2), config_.charHeight(), Qt::AlignCenter, info.hex);
		} else {
			// Draw block lowwer nibble
			QBrush brush(config_.Colors[Color::CaretBackground]);
			info.painter.setBackground(brush);
			info.painter.setPen(config_.Colors[Color::CaretText]);
			info.painter.fillRect(config_.x(info.x) + config_.ByteMargin.left() + config_.charWidth(), info.y, config_.charWidth() + config_.ByteMargin.right(), config_.byteHeight(), brush);
			QString low(info.hex[1]);
			info.painter.drawText(config_.x(info.x) + config_.ByteMargin.left() + config_.charWidth(), info.y + config_.ByteMargin.top(), config_.charWidth(2), config_.charHeight(), Qt::AlignLeft, low);
		}
	} else {
		// Draw block without data
		QBrush brush(config_.Colors[Color::CaretBackground]);
		info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
	}
}

void TextView::drawCaretFrame(const CaretDrawInfo &info)
{
	int width, x;
	//if (!info.caret_middle) {
	if (true) {
		width = config_.byteWidth() - 1;
		x = config_.x(info.x);
	} else {
		width = config_.charWidth() + config_.ByteMargin.right() - 1;
		x = config_.x(info.x) + config_.charWidth() + config_.ByteMargin.left();
	}
	info.painter.setPen(config_.Colors[Color::CaretBackground]);
	info.painter.drawRect(x, info.y, width, config_.byteHeight() - 1);
}

void TextView::drawCaretUnderbar(const CaretDrawInfo &info)
{
	int width, x;
	if (!info.caret_middle) {
		width = config_.byteWidth() - 1;
		x = config_.x(info.x);
	} else {
		width = config_.charWidth() + config_.ByteMargin.right() - 1;
		x = config_.x(info.x) + config_.ByteMargin.left() + config_.charWidth();
	}

	QBrush brush(config_.Colors[Color::CaretBackground]);
	info.painter.fillRect(x, info.y + config_.byteHeight() - 2, width, 2, brush);
}

void TextView::getLetter(int index, QString &h)
{
	//int len = getLetterLength(buff[index]);
	/*
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
	*/
}

void TextView::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) {

		cursor_->movePosition(posAt(ev->pos()), false, false);

		// Start mouse capture
		grabMouse();
	}
}

void TextView::mouseMoveEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) {
		// FIXME: move up/down automatically
		if (height() < ev->pos().y()) {
			return;
		}
		cursor_->movePosition(posAt(ev->pos()), true, false);
	}
}

void TextView::mouseReleaseEvent(QMouseEvent *)
{
	//qDebug("mouse release");

	// End mouse capture
	releaseMouse();
}

quint64 TextView::posAt(const QPoint &pos)
{
	int x = config_.XToPos(pos.x());
	int y = config_.YToLine(pos.y());

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		x = y = 0;
	}

	return qMin((cursor_->Top + y) * TextConfig::Num + x, document_->length());
}

// Enable caret blink
void TextView::setCaretBlink(bool enable)
{
	if (!config_.EnableCaret || !config_.CaretBlinkTime) {
		return;
	}
	if (enable) {
		if (cursor_->CaretTimerId == 0) {
			cursor_->CaretTimerId = startTimer(config_.CaretBlinkTime);
		}
	} else {
		if (cursor_->CaretTimerId != 0) {
			killTimer(cursor_->CaretTimerId);
			cursor_->CaretTimerId = 0;
		}
	}
}

void TextView::timerEvent(QTimerEvent *ev)
{
	if (cursor_->CaretTimerId == ev->timerId()) {
		// Caret blink
		drawCaret(cursor_->CaretVisible);
		cursor_->turnCaretVisible();
	}
}

void TextView::keyPressEvent(QKeyEvent *ev)
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

	bool keepAnchor = ev->modifiers() & Qt::SHIFT ? true : false;
	switch (ev->key()) {
	case Qt::Key_Home:
		cursor_->movePosition(0, keepAnchor, false);
		break;
	case Qt::Key_End:
		cursor_->movePosition(document_->length(), keepAnchor, false);
		break;
	case Qt::Key_Left:
		cursor_->moveRelativePosition(-1, keepAnchor, false);
		break;
	case Qt::Key_Right:
		cursor_->moveRelativePosition(1, keepAnchor, false);
		break;
	case Qt::Key_Up:
		cursor_->moveRelativePosition(-16, keepAnchor, false);
		break;
	case Qt::Key_Down:
		cursor_->moveRelativePosition(16, keepAnchor, false);
		break;
	case Qt::Key_PageUp:
		cursor_->moveRelativePosition(-16 * 15, keepAnchor, true);
		break;
	case Qt::Key_PageDown:
		cursor_->moveRelativePosition(16 * 15, keepAnchor, true);
		break;
	case Qt::Key_Backspace:
		if (cursor_->hasSelection()) {
			const quint64 pos = qMin(cursor_->Position, cursor_->PositionAnchor);
			const quint64 len = qMax(cursor_->Position, cursor_->PositionAnchor) - pos;
			removeData(pos, len);
			cursor_->moveRelativePosition(pos, false, false);
			// TODO: drawView [pos. pos+len]
			drawView();
		} else if (0 < cursor_->Position) {
			removeData(cursor_->Position - 1, 1);
			cursor_->moveRelativePosition(-1, false, false);
		}
		break;
	case Qt::Key_Insert:
		qDebug("key insert");
		cursor_->reverseInsert();
		break;
	case Qt::Key_Delete:
		if (cursor_->hasSelection()) {
			const quint64 pos = qMin(cursor_->Position, cursor_->PositionAnchor);
			const quint64 len = qMax(cursor_->Position, cursor_->PositionAnchor) - pos;
			removeData(pos, len);
			cursor_->moveRelativePosition(0, false, false);
			// TODO: drawView [pos. pos+len]
			drawView();
		} else if (cursor_->Position < document_->length()) {
			removeData(cursor_->Position, 1);
			cursor_->moveRelativePosition(0, false, false);
		}
		break;
	default:
		{
			// copy from QtCreator
			QString text = ev->text();
			for (int i = 0; i < text.length(); i++) {
			}
		}
		return;
	}
}

void TextView::changeData(quint64 pos, uchar character, bool highNibble)
{
	document_->remove(pos, 1);
	document_->insert(pos, &character, 1);
	// TODO: implement Redraw Event
	//drawView(DRAW_LINE, pos / TextConfig::Num - cursor_->Top);
	drawView();
}

void TextView::insertData(quint64 pos, uchar character)
{
	document_->insert(pos, &character, 1);
	// TODO: implement Redraw Event
	//drawViewAfter(pos);
	//drawCaret();
	drawView();
}

void TextView::removeData(quint64 pos, quint64 len)
{
	document_->remove(pos, len);
	// TODO: implement Redraw Event
	//drawViewAfter(pos);
	drawView();
}




}	// namespace
