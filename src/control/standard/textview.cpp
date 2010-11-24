
#include <QtGui>
#include <QInputContextFactory>
#include <algorithm>
#include <vector>
#include "textview.h"
#include "../util/util.h"
#include "../document.h"
#include "../highlight.h"
#include "textdecodehelper.h"
#include "caretdrawer.h"

using namespace std;

namespace Standard {

////////////////////////////////////////
// Config
TextConfig::TextConfig()
	: num_(16)
	, margin_(2, 2, 3, 3)
	, byteMargin_(0, 0, 0, 0)
	, font_("Monaco", 17)
	, fontMetrics_(font_)
	
	, EnableCaret(true)
	, CaretBlinkTime(500)
{
	// Coloring
	colors_[Color::Background] = QColor(0xEF,0xDF,0xDF);
	colors_[Color::Text] = QColor(0,0,0);
	colors_[Color::SelBackground] = QColor(0xA0,0xA0,0xFF);
	colors_[Color::SelText] = QColor(0,0,0);
	colors_[Color::CaretBackground] = QColor(0xFF, 0, 0, 200);	// + transparency

	// Font
	font_.setFixedPitch(true);

	update();
}

void TextConfig::update()
{
	x_begin.clear();
	x_end.clear();
	x_area.clear();

	// Pos
	x_begin.push_back(0);
	for (size_t i = 1; i < num_; i++) {
		x_begin.push_back(x_begin.back() + byteWidth());
	}

	// Pos of end
	for (size_t i = 0; i < x_begin.size(); i++) {
		x_end.push_back(x_begin[i] + charWidth(1));
	}
	x_end.back() += byteWidth();

	// Area
	x_area.push_back(0);
	for (size_t i = 1; i < num_; i++) {
		x_area.push_back(x_area.back() + byteWidth());
	}
	x_area.back() += byteWidth();
}

int TextConfig::drawableLines(int height) const
{
	const int y = top() + byteMargin_.top();
	return (height - y + byteHeight()) / byteHeight();
}

int TextConfig::XToPos(int x) const
{
	if (x < margin_.left()) {
		return -1;
	}

	return (int)distance(x_area.begin(), lower_bound(x_area.begin(), x_area.end(), x)) - 1;
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

TextView::TextView(QWidget *parent, ::Document *doc, Highlight *hi)
	: View(parent, doc)
	, cursor_(new Cursor(doc))
	, decode_helper_(new TextDecodeHelper(*doc, QString("Shift-JIS"), cursor_->top()))
	, caret_(CARET_BLOCK, CARET_FRAME)
	, caret_drawer_(new TextCaretDrawer(caret_, config_))
{
	// Enable keyboard input
	setFocusPolicy(Qt::WheelFocus);


	// Add document event
	connect(document_, SIGNAL(inserted(quint64, quint64)), this, SLOT(inserted(quint64, quint64)));
	connect(document_, SIGNAL(removed(quint64, quint64)), this, SLOT(removed(quint64, quint64)));

	// for Qt4.6?
	//setEnabled(true);
	//setMouseTracking(true);
	
	// Input Method
	setAttribute(Qt::WA_InputMethodEnabled);
#ifdef Q_WS_MAC
	setInputContext(QInputContextFactory::create(QString("mac"), this));
#endif

#ifdef Q_WS_WIN
	setInputContext(QInputContextFactory::create(QString("win"), this));
#endif
}

TextView::~TextView()
{
	delete decode_helper_;
	delete cursor_;
}

void TextView::resizeEvent(QResizeEvent *rs)
{
	QSize size(qMin(rs->size().width(), config_.maxWidth()), rs->size().height());
	QResizeEvent resize(size, rs->oldSize());
	View::resizeEvent(&resize);
	pix_.fill(config_.color(Color::Background));
	drawView();
}

void TextView::drawView()
{
	// FIXME: refactoring refresh event
	QPainter painter;
	painter.begin(&pix_);
	painter.setFont(config_.font());

	// TODO: draw Empty Background only

	//Q_ASSERT(static_cast<uint>(end) <= document_->length() / config_.getNum() + 1);

	// Get draw range
	int y_top = config_.top();
	int count_draw_line = config_.drawableLines(height());

	// Get top position of view
	const quint64 top = cursor_->top() * config_.getNum();
	const uint size = qMin(document_->length() - top, (quint64)config_.getNum() * count_draw_line);

	qDebug("refresh event line:%llu end:%d", cursor_->top(), count_draw_line);
	//qDebug(" pos:%llu, anchor:%llu", cursor_->position(), cursor_->anchor());

	if (size == 0) {
		return;
	}

	// Draw empty area(after end line)
	QBrush brush(config_.color(Color::Background));
	const int y_start = y_top + qMax(0, count_draw_line - 1) * config_.byteHeight();
	painter.fillRect(0, y_start, width(), height(), brush);

	// Copy from document
	if (buff_.capacity() < size) {
		buff_.resize(size);
	}
	document_->get(top, &buff_[0], size);

	// Draw lines
	drawLines(painter, top, y_top, size);

	// Update screen buffer
	const int draw_width  = qMin(width(), config_.maxWidth());
	const int draw_height = count_draw_line * config_.byteHeight();
	painter.end();
	update(0, y_top, draw_width, draw_height);
}

ColorType TextView::getColorType(const CursorSelection &c, quint64 pos)
{
	if (c.begin <= pos && pos < c.end) {
		return ColorType(Color::SelBackground, Color::SelText);
	} else {
		return ColorType(Color::Background, Color::Text);
	}
}

void TextView::drawLines(QPainter &painter, quint64 docpos, int y, uint size)
{
	TextConfig::XIterator xitr = config_.createXIterator();
	TextConfig::YIterator yitr = config_.createYIterator(y);
	const CursorSelection selection = cursor_->getSelection();
	
	// Draw loop
	for (uint index = 0; index < size; ) {
		// 印字可能な文字を描画
		uint printableBytes = decode_helper_->getPrintableBytes(index);
		if (printableBytes > 0) {
			// get data
			uchar *b = &buff_[index];
			QTextCodec::ConverterState state(QTextCodec::ConvertInvalidToNull);
			QString text = decode_helper_->getCodec()->toUnicode((char*)b, printableBytes, &state);

			int epos = qMin(*xitr + printableBytes, (uint)config_.getNum() - 1);
			QPixmap pix(QSize(config_.posWidth(*xitr, epos), config_.byteHeight()));

			QPainter letterPainter(&pix);
			letterPainter.setFont(config_.font());
			uint i = 0;
			while (i < printableBytes && *xitr + i < config_.getNum()) {
				// Set color
				ColorType color = getColorType(selection, docpos);
				QBrush brush = QBrush(config_.color(color.Background));
				letterPainter.setBackground(brush);
				letterPainter.setPen(config_.color(color.Text));

				// Draw background/text
				letterPainter.fillRect(pix.rect(), brush);
				drawText(letterPainter, text, 0, 0);

				// Copy letterPainter to painter
				painter.drawPixmap(config_.x(*xitr + i), yitr.screenY(), pix, config_.x_(i), 0, config_.posWidth(*xitr + i), pix.height());

				++i;
				++docpos;
			}

			xitr += i;
			// 次の行
			if (xitr.isNext()) {
				xitr.setNext(false);
				++yitr;

				while (i < printableBytes) {
					ColorType color = getColorType(selection, docpos);
					QBrush brush = QBrush(config_.color(color.Background));
					painter.setBackground(brush);
					painter.setPen(config_.color(color.Text));

					QString text = QString(QChar('_'));

					// Draw background
					painter.fillRect(xitr.textX(), yitr.screenY(), config_.posWidth(*xitr), config_.byteHeight(), brush);

					drawText(painter, text, xitr.textX(), yitr.screenY(), 1);

					++i;
					++docpos;
					++xitr;
				}
			}
			index += printableBytes;
		} else {
			// Set color
			ColorType color = getColorType(selection, docpos);
			QBrush brush = QBrush(config_.color(color.Background));
			painter.setBackground(brush);
			painter.setPen(config_.color(color.Text));

			// 印字不可なのでドットを描画
			QString text = QString(QChar('.'));

			// Draw
			painter.fillRect(xitr.textX(), yitr.screenY(), config_.posWidth(*xitr), config_.byteHeight(), brush);
			drawText(painter, text, xitr.textX(), yitr.screenY(), 1);

			++index;
			++xitr;
			++docpos;

			// 描画座標を次の行にする
			if (xitr.isNext()) {
				xitr.setNext(false);
				++yitr;
			}
		}
	}

	// Draw empty area(after end line)
	if (0 < *xitr && *xitr < config_.getNum()) {
		QBrush brush(config_.color(Color::Background));
		painter.fillRect(xitr.textX(), yitr.screenY(), width(), config_.byteHeight(), brush);
	}
}

inline void TextView::drawText(QPainter &painter, const QString &hex, int x, int y, int charwidth)
{
	painter.drawText(x, y, config_.charWidth(charwidth), config_.charHeight(), Qt::AlignCenter, hex);
}

inline void TextView::drawText(QPainter &painter, const QString &str, int x, int y)
{
	painter.drawText(x, y, config_.textWidth(str), config_.charHeight(), Qt::AlignCenter, str);
}

void TextView::caretDrawEvent(QPainter *painter)
{
	painter->setFont(config_.font());

	// Get caret coordinates
	const quint64 pos = cursor_->position();
	const int x = pos % config_.getNum();
	const int y = config_.top() + config_.byteHeight() * (pos / config_.getNum() - cursor_->top());

	const bool caret_middle = pos < document_->length();
	caret_drawer_->drawCaret(painter, x, y, caret_.visible(), caret_middle);
}

void TextView::drawCaret(bool visible)
{
	drawCaret(visible, cursor_->position());
}

void TextView::drawCaret(bool visible, quint64 pos)
{
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

void TextView::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) {
		qDebug("mosue press");

		movePosition(posAt(ev->pos()), false, false);

		// Start mouse capture
		//grabMouse();
	}
}

void TextView::mouseMoveEvent(QMouseEvent *ev)
{
	//qDebug("mouse move :%d", ev->button());
	//qDebug("mosue move");
	// FIXME: move up/down automatically
	if (height() < ev->pos().y()) {
		return;
	}
	movePosition(posAt(ev->pos()), true, false);
}

void TextView::mouseReleaseEvent(QMouseEvent *)
{
	//qDebug("mouse release");

	// End mouse capture
	//releaseMouse();
}

quint64 TextView::posAt(const QPoint &pos) const
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
void TextView::setCaretBlink(bool enable)
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

void TextView::timerEvent(QTimerEvent *ev)
{
	if (caret_.timerId() == ev->timerId()) {
		drawCaret(caret_.visible());
		caret_.inverseVisible();
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
		movePosition(0, keepAnchor, false);
		break;
	case Qt::Key_End:
		movePosition(document_->length(), keepAnchor, false);
		break;
	case Qt::Key_Left:
		moveRelativePosition(-1, keepAnchor, false);
		break;
	case Qt::Key_Right:
		moveRelativePosition(1, keepAnchor, false);
		break;
	case Qt::Key_Up:
		moveRelativePosition((qint64)-1 * config_.getNum(), keepAnchor, false);
		break;
	case Qt::Key_Down:
		moveRelativePosition((qint64)config_.getNum(), keepAnchor, false);
		break;
	case Qt::Key_PageUp:
		moveRelativePosition((qint64)-1 * config_.getNum() * 15, keepAnchor, true);
		break;
	case Qt::Key_PageDown:
		moveRelativePosition((qint64)config_.getNum() * 15, keepAnchor, true);
		break;
	case Qt::Key_Backspace:
		if (cursor_->hasSelection()) {
			const quint64 pos = qMin(cursor_->position(), cursor_->anchor());
			const quint64 len = qMax(cursor_->position(), cursor_->anchor()) - pos;
			removeData(pos, len);
			moveRelativePosition(pos, false, false);
			// TODO: drawView [pos. pos+len]
			drawView();
		} else if (0 < cursor_->position()) {
			removeData(cursor_->position() - 1, 1);
			moveRelativePosition(-1, false, false);
		}
		break;
	case Qt::Key_Insert:
		qDebug("key insert");
		cursor_->reverseInsert();
		break;
	case Qt::Key_Delete:
		if (cursor_->hasSelection()) {
			const quint64 pos = qMin(cursor_->position(), cursor_->anchor());
			const quint64 len = qMax(cursor_->position(), cursor_->anchor()) - pos;
			removeData(pos, len);
			moveRelativePosition(0, false, false);
			// TODO: drawView [pos. pos+len]
			drawView();
		} else if (cursor_->position() < document_->length()) {
			removeData(cursor_->position(), 1);
			moveRelativePosition(0, false, false);
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

void TextView::inputMethodEvent(QInputMethodEvent *ev)
{
	qDebug() << "input method" << ev->commitString();
	qDebug() << "input method" << ev->preeditString();
}

QVariant TextView::inputMethodQuery(Qt::InputMethodQuery query) const
{
}

void TextView::movePosition(quint64 pos, bool sel, bool holdViewPos)
{
	Q_ASSERT(pos <= document_->length());
	
	const quint64 oldTop = cursor_->top();
	const quint64 oldPos = cursor_->position();
	const quint64 oldPosAnchor = cursor_->anchor();
	const bool oldSelection = cursor_->hasSelection();

	// movePosition
	cursor_->movePosition(this, pos, sel, holdViewPos);

	// Redraw view
	if (cursor_->top() == oldTop) {
		if (!sel && oldSelection) {
			// Clear selection
			redrawSelection(qMin(oldPos, oldPosAnchor), qMax(oldPos, oldPosAnchor));
		} else if (cursor_->position() != oldPos) {
			// Draw/Redraw selection
			const quint64 begin = qMin(qMin(cursor_->position(), cursor_->anchor()), oldPos);
			const quint64 end   = qMax(qMax(cursor_->position(), cursor_->anchor()), oldPos);
			redrawSelection(begin, end);
		}
		// TODO: Clear old caret only
		drawView();
	} else {
		drawView();
	}

	drawCaret();
}


void TextView::moveRelativePosition(qint64 pos, bool sel, bool holdViewPos)
{
	movePosition(cursor_->getRelativePosition(pos), sel, holdViewPos);
}

void TextView::redrawSelection(quint64 begin, quint64 end)
{
	//qDebug("redrawSelection %llu, %llu, Top:%llu", begin, end, Top);
	begin /= config_.getNum();
	end   /= config_.getNum();

	// FIXIME: redraw [beginLine, endLine]
	drawView();
}

void TextView::changeData(quint64 pos, uchar character)
{
	document_->remove(pos, 1);
	document_->insert(pos, &character, 1);
	// TODO: implement Redraw Event
	//drawView();
}

void TextView::insertData(quint64 pos, uchar character)
{
	document_->insert(pos, &character, 1);
	// TODO: implement Redraw Event
	//drawCaret();
	//drawView();
}

void TextView::removeData(quint64 pos, quint64 len)
{
	document_->remove(pos, len);
	// TODO: implement Redraw Event
	//drawView();
}

void TextView::inserted(quint64 pos, quint64 len)
{
	// TODO: lazy redraw
	//drawView(DRAW_AFTER, pos / config_.getNum() - cursor_-> Top);
	drawView();
}


void TextView::removed(quint64 pos, quint64 len)
{
	// TODO: lazy redraw
	//drawView(DRAW_AFTER, pos / config_.getNum() - cursor_-> Top);
	drawView();
}



}	// namespace
