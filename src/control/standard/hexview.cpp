
#include <QtGui>
#include <algorithm>
#include <vector>
#include "hexview.h"
#include "hexcursor.h"
#include "../document.h"
#include "../highlight.h"

using namespace std;

namespace Standard {

////////////////////////////////////////
// Config
HexConfig::HexConfig()
	: Num(16)
	, Margin(2, 2, 3, 3)
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
	Colors[Color::CaretBackground] = QColor(0xFF, 0, 0, 200);	// + transparency
	Colors[Color::CaretText] = QColor(0xFF,0xFF,0xFF);

	// Font
	Font.setFixedPitch(true);

	update();
}

void HexConfig::update()
{
	x_begin.clear();
	x_end.clear();
	x_area.clear();

	// Pos
	x_begin.push_back(Margin.left() + ByteMargin.left());
	for (int i = 1; i < Num; i++) {
		x_begin.push_back(x_begin.back() + byteWidth());
	}

	// Pos of end
	for (int i = 0; i < Num; i++) {
		x_end.push_back(x_begin[i] + charWidth(2) + ByteMargin.right());
	}

	// Area
	x_area.push_back(Margin.left() + ByteMargin.left());
	for (int i = 1; i < getNumV(); i++) {
		x_area.push_back(x_area.back() + byteWidth());
	}
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

	return (int)distance(x_area.begin(), lower_bound(x_area.begin(), x_area.end(), x)) - 1;
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
	, cursor_(new HexCursor(doc, this))
	, caret_(CARET_BLOCK, CARET_FRAME)
{
	// Enable keyboard input
	setFocusPolicy(Qt::WheelFocus);

	//setMouseTracking(true);
}

HexView::~HexView()
{
	delete cursor_;
}

void HexView::resizeEvent(QResizeEvent *rs)
{
	QSize size(qMin(rs->size().width(), config_.maxWidth()), rs->size().height());
	QResizeEvent resize(size, rs->oldSize());
	View::resizeEvent(&resize);
	pix_.fill(config_.Colors[Color::Background]);
	drawView();
}

void HexView::drawView(DrawMode mode, int line_start, int end)
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
	Q_ASSERT(static_cast<uint>(line_start) <= document_->length() / config_.getNum() + 1);
	Q_ASSERT(0 <= end);
	Q_ASSERT(static_cast<uint>(end) <= document_->length() / config_.getNum() + 1);

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
	const quint64 top = (cursor_->Top + line_start) * config_.getNum();
	const uint size = qMin(document_->length() - top, (quint64)config_.getNum() * count_draw_line);
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
	const int x_count_max = (width() - config_.Margin.left()) / config_.byteWidth() + 1;
	drawLines(painter, top, y_top, 0, x_count_max, size);

	// Update screen buffer
	const int draw_width  = qMin(width(), config_.maxWidth());
	const int draw_height = count_draw_line * config_.byteHeight();
	painter.end();
	update(0, y_top, draw_width, draw_height);

	//qDebug("HexView: emit viewDrawed(mode, line_start, end);");
	emit viewDrawed(mode, line_start, end);
}

//inline void HexView::drawViewAfter(quint64 pos)
//{
//	drawView(DRAW_AFTER, pos / config_.getNum() - cursor_-> Top);
//}


ColorType HexView::getColorType(const CursorSelection &c, quint64 pos)
{
	if (c.begin <= pos && pos < c.end) {
		return ColorType(Color::SelBackground, Color::SelText);
	} else {
		return ColorType(Color::Background, Color::Text);
	}
}

void HexView::drawLines(QPainter &painter, quint64 docpos, int y, int x_begin, int x_end, uint size)
{
	HexConfig::XIterator xitr = config_.createXIterator();
	HexConfig::YIterator yitr = config_.createYIterator(y);
	const CursorSelection selection = cursor_->getSelection();
	QString hex;
	hex.resize(2);

	// Draw loop
	for (uint index = 0; index < size; ++index) {
		if (x_begin <= *xitr && *xitr < x_end) {
			// Set color
			ColorType color = getColorType(selection, docpos++);
			QBrush brush = QBrush(config_.Colors[color.Background]);
			painter.setBackground(brush);
			painter.setPen(config_.Colors[color.Text]);

			// Draw background
			painter.fillRect(xitr.getScreenX(), *yitr, config_.byteWidth(), config_.byteHeight(), brush);

			// Draw text
			byteToHex(buff_[index], hex);
			drawText(painter, hex, xitr.getTextX(), yitr.getScreenY());
		}

		// Move next line
		++xitr;
		//if (*xitr == 0) {
		if (xitr.is_next_flag()) {
			xitr.set_next_flag(false);
			++yitr;
		}
	}

	// Draw empty area(after end line)
	if (0 < *xitr && *xitr < x_end && *xitr < config_.getNum()) {
		QBrush brush(config_.Colors[Color::Background]);
		painter.fillRect(xitr.getScreenX(), *yitr, width(), config_.byteHeight(), brush);
	}
}

inline void HexView::drawText(QPainter &painter, const QString &hex, int x, int y)
{
	painter.drawText(x, y, config_.charWidth(2), config_.charHeight(), Qt::AlignCenter, hex);
}

void HexView::drawCaret(bool visible)
{
	drawCaret(visible, cursor_->Position);
}

void HexView::drawCaret(bool visible, quint64 pos)
{
	// Check out of range
	if (!(config_.top() + config_.byteHeight() < height())) {
		return;
	}

	// Redraw line
	const quint64 line = cursor_->Position / config_.getNum();
	if (cursor_->Top <= line && line - cursor_->Top < static_cast<unsigned int>(config_.drawableLines(height()))) {
		drawView(DRAW_LINE, line - cursor_->Top);
	}

	// Shape
	const CaretShape shape = caret_.getShape(visible);
	if (shape == CARET_NONE) {
		return;
	}

	// Begin paint
	QPainter painter;
	painter.begin(&pix_);
	painter.setFont(config_.Font);

	// Get caret coordinates
	const int x = pos % config_.getNum();
	const int y = config_.top() + config_.byteHeight() * (pos / config_.getNum() - cursor_->Top);

	// Draw shape
	drawCaretShape(CaretDrawInfo(painter, shape, pos, x, y, pos < document_->length()));

	// Finish paint and update screen buffer
	painter.end();
	update(config_.x(x), y, config_.byteWidth(), config_.charHeight());
}

void HexView::drawCaretShape(CaretDrawInfo info)
{
	if (info.caret_middle) {
		// Copy from document
		uchar data;
		document_->get(info.pos, &data, 1);

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
	default:
		;
	}
}

void HexView::drawCaretLine(const CaretDrawInfo &info)
{
	int x;
	if (cursor_->HighNibble || !info.caret_middle) {
		x = config_.x(info.x);
	} else {
		x = config_.x(info.x) + config_.ByteMargin.left() + config_.charWidth();
	}
	QBrush brush(config_.Colors[Color::CaretBackground]);
	info.painter.fillRect(x, info.y, 2, config_.byteHeight(), brush);
}

void HexView::drawCaretBlock(const CaretDrawInfo &info)
{
	if (info.caret_middle) {
		if (cursor_->HighNibble || cursor_->hasSelection()) {
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

void HexView::drawCaretFrame(const CaretDrawInfo &info)
{
	int width, x;
	if (cursor_->HighNibble || !info.caret_middle) {
		width = config_.byteWidth() - 1;
		x = config_.x(info.x);
	} else {
		width = config_.charWidth() + config_.ByteMargin.right() - 1;
		x = config_.x(info.x) + config_.charWidth() + config_.ByteMargin.left();
	}
	info.painter.setPen(config_.Colors[Color::CaretBackground]);
	info.painter.drawRect(x, info.y, width, config_.byteHeight() - 1);
}

void HexView::drawCaretUnderbar(const CaretDrawInfo &info)
{
	int width, x;
	if (cursor_->HighNibble || !info.caret_middle) {
		width = config_.byteWidth() - 1;
		x = config_.x(info.x);
	} else {
		width = config_.charWidth() + config_.ByteMargin.right() - 1;
		x = config_.x(info.x) + config_.ByteMargin.left() + config_.charWidth();
	}

	QBrush brush(config_.Colors[Color::CaretBackground]);
	info.painter.fillRect(x, info.y + config_.byteHeight() - 2, width, 2, brush);
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
		//qDebug("mouse press");

		cursor_->HighNibble = true;
		cursor_->movePosition(posAt(ev->pos()), false, false);

		// Start mouse capture
		grabMouse();
	}
}

void HexView::mouseMoveEvent(QMouseEvent *ev)
{
	//qDebug("mouse move");
	// FIXME: move up/down automatically
	if (height() < ev->pos().y()) {
		return;
	}
	cursor_->movePosition(posAt(ev->pos()), true, false);
}

void HexView::mouseReleaseEvent(QMouseEvent *)
{
	//qDebug("mouse release");

	// End mouse capture
	releaseMouse();
}

quint64 HexView::posAt(const QPoint &pos) const
{
	int x = config_.XToPos(pos.x());
	int y = config_.YToLine(pos.y());

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		x = y = 0;
	}

	return qMin((cursor_->Top + y) * config_.getNum() + x, document_->length());
}

// Enable caret blink
void HexView::setCaretBlink(bool enable)
{
	if (!config_.EnableCaret || !config_.CaretBlinkTime) {
		return;
	}
	if (enable) {
		if (caret_.getTimerId() == 0) {
			caret_.setTimerId(startTimer(config_.CaretBlinkTime));
		}
	} else {
		if (caret_.getTimerId() != 0) {
			killTimer(caret_.getTimerId());
			caret_.setTimerId(0);
		}
	}
}

void HexView::timerEvent(QTimerEvent *ev)
{
	if (caret_.getTimerId() == ev->timerId()) {
		drawCaret(caret_.getVisible());
		caret_.inverseVisible();
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

	bool keepAnchor = ev->modifiers() & Qt::SHIFT ? true : false;
	switch (ev->key()) {
	case Qt::Key_Home:
		cursor_->HighNibble = true;
		cursor_->movePosition(0, keepAnchor, false);
		break;
	case Qt::Key_End:
		cursor_->HighNibble = true;
		cursor_->movePosition(document_->length(), keepAnchor, false);
		break;
	case Qt::Key_Left:
		cursor_->HighNibble = true;
		cursor_->moveRelativePosition(-1, keepAnchor, false);
		break;
	case Qt::Key_Right:
		cursor_->HighNibble = true;
		cursor_->moveRelativePosition(1, keepAnchor, false);
		break;
	case Qt::Key_Up:
		cursor_->HighNibble = true;
		cursor_->moveRelativePosition((qint64)-1 * config_.getNum(), keepAnchor, false);
		break;
	case Qt::Key_Down:
		cursor_->HighNibble = true;
		cursor_->moveRelativePosition((qint64)config_.getNum(), keepAnchor, false);
		break;
	case Qt::Key_PageUp:
		cursor_->HighNibble = true;
		cursor_->moveRelativePosition((qint64)-1 * config_.getNum() * 15, keepAnchor, true);
		break;
	case Qt::Key_PageDown:
		cursor_->HighNibble = true;
		cursor_->moveRelativePosition((qint64)config_.getNum() * 15, keepAnchor, true);
		break;
	case Qt::Key_Backspace:
		if (cursor_->hasSelection()) {
			const quint64 pos = qMin(cursor_->Position, cursor_->PositionAnchor);
			const quint64 len = qMax(cursor_->Position, cursor_->PositionAnchor) - pos;
			removeData(pos, len);
			cursor_->moveRelativePosition(pos, false, false);
			// TODO: drawView [pos. pos+len]
			drawView();
			cursor_->HighNibble = true;
		} else if (0 < cursor_->Position) {
			removeData(cursor_->Position - 1, 1);
			cursor_->moveRelativePosition(-1, false, false);
			cursor_->HighNibble = true;
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
			cursor_->HighNibble = true;
		} else if (cursor_->Position < document_->length()) {
			removeData(cursor_->Position, 1);
			cursor_->moveRelativePosition(0, false, false);
			cursor_->HighNibble = true;
		}
		break;
	default:
		{
			// copy from QtCreator
			QString text = ev->text();
			for (int i = 0; i < text.length(); i++) {
				QChar c = text.at(i).toLower();
				int nibble = -1;
				if (c.unicode() >= 'a' && c.unicode() <= 'f') {
					nibble = c.unicode() - 'a' + 10;
				} else if (c.unicode() >= '0' && c.unicode() <= '9') {
					nibble = c.unicode() - '0';
				}
				if (nibble < 0) {
					continue;
				}
				//if (cursor_->Insert && cursor_->HighNibble) {
				if (false) {
					// Inserte mode
					quint64 pos = qMin(cursor_->Position, cursor_->PositionAnchor);

					// Replace data if selected
					if (cursor_->hasSelection()) {
						// TODO: Support Undo
						// Off redrawing temporary for redrawing on insertion
						document_->remove(pos, qMax(cursor_->Position, cursor_->PositionAnchor) - pos);
						cursor_->Position = pos;
						cursor_->resetAnchor();
						// TODO: remove and refresh collectly
						//cursor_->moveRelativePosition(0, false, false);
					}

					insertData(pos, nibble << 4);
					cursor_->HighNibble = false;
					drawCaret();
				} else if (cursor_->Position < document_->length()) {
					// Ovewrite mode
					uchar currentCharacter;
					document_->get(cursor_->Position, &currentCharacter, 1);
					if (cursor_->HighNibble) {
						changeData(cursor_->Position, (nibble << 4) + (currentCharacter & 0x0f), true);
						cursor_->HighNibble = false;
						drawCaret();
					} else {
						cursor_->moveRelativePosition(1, false, false);
						changeData(cursor_->Position - 1, nibble + (currentCharacter & 0xf0));
					}
				} else {
					break;
				}
			}
		}
		return;
	}
}

void HexView::changeData(quint64 pos, uchar character, bool highNibble)
{
	document_->remove(pos, 1);
	document_->insert(pos, &character, 1);
	cursor_->HighNibble = !highNibble;
	// TODO: implement Redraw Event
	//drawView(DRAW_LINE, pos / config_.getNum() - cursor_->Top);
	drawView();
}

void HexView::insertData(quint64 pos, uchar character)
{
	document_->insert(pos, &character, 1);
	// TODO: implement Redraw Event
	//drawViewAfter(pos);
	//drawCaret();
	drawView();
}

void HexView::removeData(quint64 pos, quint64 len)
{
	document_->remove(pos, len);
	// TODO: implement Redraw Event
	//drawViewAfter(pos);
	drawView();
}




}	// namespace
