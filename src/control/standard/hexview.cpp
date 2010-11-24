#include <QtGui>
#include <algorithm>
#include <vector>
#include "hexview.h"
#include "../util/util.h"
#include "../document.h"
#include "../highlight.h"
#include "caretdrawer.h"

using namespace std;

namespace Standard {

////////////////////////////////////////
// Config
HexConfig::HexConfig()
	: num_(16)
	, margin_(2, 2, 3, 3)
	, byteMargin_(3, 0, 2, 0)
	, font_("Monaco", 17)
	, charWidth_(0)
	, fontMetrics_(font_)

	, EnableCaret(true)
	, CaretBlinkTime(500)
{
	// Coloring
	colors_[Color::Background] = QColor(0xEF,0xEF,0xEF);
	colors_[Color::Text] = QColor(0,0,0);
	colors_[Color::SelBackground] = QColor(0xA0,0xA0,0xFF);
	colors_[Color::SelText] = QColor(0,0,0);
	colors_[Color::CaretBackground] = QColor(0xFF, 0, 0, 200);	// + transparency

	// Font
	font_.setFixedPitch(true);

	update();
}

void HexConfig::update()
{

	// get charWidth
	for (int i = 0; i < 16; i++) {
		QChar ch(util::itohex(i));
		charWidth_ = qMax(charWidth_, fontMetrics_.boundingRect(ch).width());
	}

	x_begin.clear();
	x_end.clear();
	x_area.clear();

	// Pos
	x_begin.push_back(margin_.left() + byteMargin_.left());
	for (size_t i = 1; i < num_; i++) {
		x_begin.push_back(x_begin.back() + byteWidth());
	}

	// Pos of end
	for (size_t i = 0; i < num_; i++) {
		x_end.push_back(x_begin[i] + charWidth(2) + byteMargin_.right());
	}

	// Area
	x_area.push_back(margin_.left() + byteMargin_.left());
	for (size_t i = 1; i < getNumV(); i++) {
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
	if (x < margin_.left()) {
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

HexView::HexView(QWidget *parent, ::Document *doc, Highlight *)
	: View(parent, doc)
	, cursor_(new Cursor(doc))
	, caret_(CARET_BLOCK, CARET_FRAME)
	, caret_drawer_(new HexCaretDrawer(config_, cursor_))
	, keyboard_(new Keyboard(doc, this))
{
	// Enable keyboard input
	setFocusPolicy(Qt::WheelFocus);

	// Add document event
	QObject::connect(document_, SIGNAL(inserted(quint64, quint64)), this, SLOT(inserted(quint64, quint64)));
	QObject::connect(document_, SIGNAL(removed(quint64, quint64)), this, SLOT(removed(quint64, quint64)));

	// Add cursor event
	QObject::connect(cursor_, SIGNAL(topChanged(quint64)), this, SLOT(topChanged(quint64)));
	QObject::connect(cursor_, SIGNAL(positionChanged(quint64, quint64)), this, SLOT(positionChanged(quint64, quint64)));
	QObject::connect(cursor_, SIGNAL(insertChanged(bool)), this, SLOT(insertChanged(bool)));
	QObject::connect(cursor_, SIGNAL(selectionUpdate(quint64, quint64)), this, SLOT(selectionUpdate(quint64, quint64)));

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
	pix_.fill(config_.color(Color::Background));
	drawView();
}

void HexView::drawView(DrawMode mode, int line_start, int end)
{
	//qDebug("refresh event mode:%d line:%d end:%d", mode, line_start, end);
	//qDebug(" pos:%llu, anchor:%llu", cursor_->position(), cursor_->anchor());

	// FIXME: refactoring refresh event
	QPainter painter;
	painter.begin(&pix_);
	painter.setFont(config_.font());

	if (!document_->length()) {
		// TODO: draw Empty Background only
		QBrush brush(config_.color(Color::Background));
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
	const quint64 top = (cursor_->top() + line_start) * config_.getNum();
	const uint size = qMin(document_->length() - top, (quint64)config_.getNum() * count_draw_line);
	if (size == 0) {
		return;
	}

	// Draw empty area(after end line)
	if (mode == DRAW_ALL || mode == DRAW_AFTER) {
		//qDebug("draw empty area DRAW_ALL or DRAW_AFTER");
		QBrush brush(config_.color(Color::Background));
		const int y_start = y_top + qMax(0, count_draw_line - 1) * config_.byteHeight();
		painter.fillRect(0, y_start, width(), height(), brush);
	}

	// Copy from document
	if (buff_.capacity() < size) {
		buff_.resize(size);
	}
	document_->get(top, &buff_[0], size);

	// Get selectead area
	const int x_count_max = (width() - config_.margin().left()) / config_.byteWidth() + 1;
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
			ColorType color = selection.color(docpos++);
			QBrush brush = QBrush(config_.color(color.Background));
			painter.setBackground(brush);
			painter.setPen(config_.color(color.Text));

			// Draw background
			painter.fillRect(xitr.screenX(), *yitr, config_.byteWidth(), config_.byteHeight(), brush);

			// Draw text
			byteToHex(buff_[index], hex);
			drawText(painter, hex, xitr.textX(), yitr.screenY());
		}

		// Move next line
		++xitr;
		//if (*xitr == 0) {
		if (xitr.isNext()) {
			xitr.setNext(false);
			++yitr;
		}
	}

	// Draw empty area(after end line)
	if (0 < *xitr && *xitr < x_end && static_cast<uint>(*xitr) < config_.getNum()) {
		QBrush brush(config_.color(Color::Background));
		painter.fillRect(xitr.screenX(), *yitr, width(), config_.byteHeight(), brush);
	}
}

inline void HexView::drawText(QPainter &painter, const QString &hex, int x, int y)
{
	painter.drawText(x, y, config_.charWidth(2), config_.charHeight(), Qt::AlignCenter, hex);
}


void HexView::caretDrawEvent(QPainter *painter)
{
	painter->setFont(config_.font());

	// Get caret coordinates
	const quint64 pos = cursor_->position();
	const int x = pos % config_.getNum();
	const int y = config_.top() + config_.byteHeight() * (pos / config_.getNum() - cursor_->top());

	const bool caret_middle = pos < document_->length();

	CaretDrawInfo info(*painter, caret_.currentShape(), pos, x, y, caret_middle);
	caret_drawer_->drawCaret(info);
}

void HexView::drawCaret()
{
	quint64 pos = cursor_->position();

	// Check out of range
	if (!(config_.top() + config_.byteHeight() < height())) {
		return;
	}

	// Get caret coordinates
	const int x = pos % config_.getNum();
	const int y = config_.top() + config_.byteHeight() * (pos / config_.getNum() - cursor_->top());
	update(config_.x(x), y, width() - config_.x(x), config_.byteHeight());
	return;
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

		cursor_->setNibble(true);
		cursor_->movePosition(this, posAt(ev->pos()), false, false);

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
	cursor_->movePosition(this, posAt(ev->pos()), true, false);
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

	return qMin((cursor_->top() + y) * config_.getNum() + x, document_->length());
}

// Enable caret blink
void HexView::setCaretBlink(bool enable)
{
	if (!config_.EnableCaret || !config_.CaretBlinkTime) {
		return;
	}
	if (enable) {
		if (caret_.timerId() == 0) {
			caret_.setTimerId(startTimer(config_.CaretBlinkTime));
		}
	} else {
		if (caret_.timerId() != 0) {
			killTimer(caret_.timerId());
			caret_.setTimerId(0);
		}
	}
}

void HexView::timerEvent(QTimerEvent *ev)
{
	if (caret_.timerId() == ev->timerId()) {
		drawCaret();
		caret_.inverseVisible();
	}
}

void HexView::keyPressEvent(QKeyEvent *ev)
{
	qDebug() << "keyPress" << ev->text();
	keyboard_->keyPressEvent(ev);
}

void HexView::moveRelativePosition(qint64 pos, bool sel, bool holdViewPos)
{
	cursor_->movePosition(this, cursor_->getRelativePosition(pos), sel, holdViewPos);
}

void HexView::redrawSelection(quint64 begin, quint64 end)
{
	//qDebug("redrawSelection %llu, %llu, Top:%llu", begin, end, Top);
	begin /= config_.getNum();
	end   /= config_.getNum();

	const int beginLine = qMax(begin, cursor_->top()) - cursor_->top();
	const int endLine   = qMax(end, cursor_->top()) - cursor_->top();

	//qDebug("redrawSelection %d, %d, Top:%llu", beginLine, endLine, Top);
	drawView(DRAW_RANGE, beginLine, endLine + 1);
}

void HexView::inserted(quint64 pos, quint64 len)
{
	// TODO: lazy redraw
	//drawViewAfter(pos);
	drawView();
}


void HexView::removed(quint64 pos, quint64 len)
{
	// TODO: lazy redraw
	//drawViewAfter(pos);
	drawView();
}

void HexView::topChanged(quint64 top)
{
	drawView();
}

void HexView::positionChanged(quint64 old, quint64 pos)
{
	// 1. pos changed(cursor only)
	// 2. selectino redraw

	// FIXME: optimize update area
	if (false) {	// TODO: check cursor line
		drawView();
	} else {
		update(0, 0, width(), height());
	}
}

void HexView::insertChanged(bool)
{
	// FIXME: optimize update area
	// update curosr pos
	update(0, 0, width(), height());
}

void HexView::selectionUpdate(quint64 begin, quint64 end)
{
	redrawSelection(begin, end);
}




}	// namespace
