
#include <QtGui>
#include <QInputContextFactory>
#include <algorithm>
#include <vector>
#include "textview.h"
#include "../util/util.h"
#include "../document.h"
#include "textdecodehelper.h"
#include "caretdrawer.h"

using namespace std;

namespace Standard {

////////////////////////////////////////
// Config
TextConfig::TextConfig(Global *global)
	: global_(global)
	, byteMargin_(0, 0, 0, 0)
{
	// Coloring
	colors_[Color::Background] = QColor(0xEF,0xDF,0xDF, 0);
	colors_[Color::Text] = QColor(0,0,0);
	colors_[Color::SelBackground] = QColor(0xA0,0xA0,0xFF, 170);
	colors_[Color::SelText] = QColor(0,0,0);
	colors_[Color::CaretBackground] = QColor(0xFF, 0, 0, 200);	// + transparency

	update();
}

void TextConfig::update()
{
	x_begin.clear();
	x_end.clear();
	x_area.clear();

	// Pos
	x_begin.push_back(0);
	for (size_t i = 1; i < num(); i++) {
		x_begin.push_back(x_begin.back() + byteWidth());
	}

	// Pos of end
	for (size_t i = 0; i < x_begin.size(); i++) {
		x_end.push_back(x_begin[i] + charWidth(1));
	}
	x_end.back() += byteWidth();

	// Area
	x_area.push_back(0);
	for (size_t i = 1; i < num(); i++) {
		x_area.push_back(x_area.back() + byteWidth());
	}
	x_area.back() += byteWidth();
}

int TextConfig::XToPos(int x) const
{
	if (x < margin().left()) {
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

TextView::TextView(QWidget *parent, Global *global)
	: View(parent)
	, global_(global)
	, document_(global->document())
	, config_(global)
	, cursor_(new Cursor)
	, decode_helper_(new TextDecodeHelper(*document_, QString("Shift-JIS"), cursor_->top()))
	, caret_(CARET_BLOCK, CARET_FRAME)
{
	// Enable keyboard input
	setFocusPolicy(Qt::WheelFocus);


	// Add document event
	connect(document_, SIGNAL(inserted(quint64, quint64)), this, SLOT(inserted(quint64, quint64)));
	connect(document_, SIGNAL(removed(quint64, quint64)), this, SLOT(removed(quint64, quint64)));

	// Add cursor event
	QObject::connect(cursor_, SIGNAL(topChanged(quint64)), this, SLOT(topChanged(quint64)));
	QObject::connect(cursor_, SIGNAL(positionChanged(quint64, quint64)), this, SLOT(positionChanged(quint64, quint64)));
	QObject::connect(cursor_, SIGNAL(insertChanged(bool)), this, SLOT(insertChanged(bool)));
	QObject::connect(cursor_, SIGNAL(selectionUpdate(quint64, quint64)), this, SLOT(selectionUpdate(quint64, quint64)));

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

void TextView::paintEvent(QPaintEvent*)
{
	// FIXME: refactoring
	drawView();
}

void TextView::drawView()
{
	// FIXME: refactoring refresh event
	QPainter painter(this);
	painter.setFont(config_.font());

	// TODO: draw Empty Background only

	//Q_ASSERT(static_cast<uint>(end) <= document_->length() / config_.num() + 1);

	// Get draw range
	int y_top = config_.top();
	int count_draw_line = global_->config().drawableLines(height());

	// Get top position of view
	const quint64 top = cursor_->top() * config_.num();
	const uint size = qMin(document_->length() - top, (quint64)config_.num() * count_draw_line);

	qDebug("refresh event line:%llu end:%d", cursor_->top(), count_draw_line);
	//qDebug(" pos:%llu, anchor:%llu", cursor_->position(), cursor_->anchor());

	if (size == 0) {
		return;
	}

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

			uint i = 0;
			while (i < printableBytes && *xitr + i < config_.num()) {
				// Set color
				ColorType color = selection.color(docpos);
				QBrush brush = QBrush(config_.color(color.Background));
				painter.setBackground(brush);
				painter.setPen(config_.color(color.Text));

				// Draw background/text
				QRect rect(config_.x(*xitr + i), yitr.screenY(), config_.posWidth(*xitr + i), config_.byteHeight());
				painter.setClipRegion(rect);
				painter.setClipping(true);
				painter.fillRect(rect, brush);
				drawText(painter, text, config_.x(*xitr), yitr.screenY());
				painter.setClipping(false);

				++i;
				++docpos;
			}

			xitr += i;
			// 次の行
			if (xitr.isNext()) {
				xitr.setNext(false);
				++yitr;

				while (i < printableBytes) {
					ColorType color = selection.color(docpos);
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
			ColorType color = selection.color(docpos);
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
}

inline void TextView::drawText(QPainter &painter, const QString &hex, int x, int y, int charwidth)
{
	painter.drawText(x, y, config_.charWidth(charwidth), config_.charHeight(), Qt::AlignCenter, hex);
}

inline void TextView::drawText(QPainter &painter, const QString &str, int x, int y)
{
	painter.drawText(x, y, config_.textWidth(str), config_.charHeight(), Qt::AlignCenter, str);
}

void TextView::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) {
		qDebug("mosue press");

		cursor_->movePosition(global_, height(), posAt(ev->pos()), false, false);

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
	cursor_->movePosition(global_, height(), posAt(ev->pos()), true, false);
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

	return qMin((cursor_->top() + y) * config_.num() + x, document_->length());
}

CaretDrawer * TextView::createCaretWidget()
{
	return new TextCaretDrawer(config_, cursor_, document_);
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
		cursor_->movePosition(global_, height(), 0, keepAnchor, false);
		break;
	case Qt::Key_End:
		cursor_->movePosition(global_, height(), document_->length(), keepAnchor, false);
		break;
	case Qt::Key_Left:
		moveRelativePosition(-1, keepAnchor, false);
		break;
	case Qt::Key_Right:
		moveRelativePosition(1, keepAnchor, false);
		break;
	case Qt::Key_Up:
		moveRelativePosition((qint64)-1 * config_.num(), keepAnchor, false);
		break;
	case Qt::Key_Down:
		moveRelativePosition((qint64)config_.num(), keepAnchor, false);
		break;
	case Qt::Key_PageUp:
		moveRelativePosition((qint64)-1 * config_.num() * 15, keepAnchor, true);
		break;
	case Qt::Key_PageDown:
		moveRelativePosition((qint64)config_.num() * 15, keepAnchor, true);
		break;
	case Qt::Key_Backspace:
		if (cursor_->hasSelection()) {
			const quint64 pos = qMin(cursor_->position(), cursor_->anchor());
			const quint64 len = qMax(cursor_->position(), cursor_->anchor()) - pos;
			removeData(pos, len);
			moveRelativePosition(pos, false, false);
			// TODO: drawView [pos. pos+len]
			//drawView();
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
			//drawView();
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

//QVariant TextView::inputMethodQuery(Qt::InputMethodQuery query) const
//{
//}

void TextView::moveRelativePosition(qint64 pos, bool sel, bool holdViewPos)
{
	cursor_->movePosition(global_, height(), cursor_->getRelativePosition(pos, document_), sel, holdViewPos);
}

void TextView::redrawSelection(quint64 begin, quint64 end)
{
	//qDebug("redrawSelection %llu, %llu, Top:%llu", begin, end, Top);
	begin /= config_.num();
	end   /= config_.num();

	// FIXIME: redraw [beginLine, endLine]
	//drawView();
	update();
}

void TextView::removeData(quint64 pos, quint64 len)
{
	document_->remove(pos, len);
	// TODO: implement Redraw Event
	//drawView();
	update();
}

void TextView::inserted(quint64 pos, quint64 len)
{
	// TODO: lazy redraw
	//drawView(DRAW_AFTER, pos / config_.num() - cursor_-> Top);
	//drawView();
	update();
}


void TextView::removed(quint64 pos, quint64 len)
{
	// TODO: lazy redraw
	//drawView(DRAW_AFTER, pos / config_.num() - cursor_-> Top);
	update();
}


void TextView::topChanged(quint64 top)
{
	//drawView();
	update();
}

void TextView::positionChanged(quint64 old, quint64 pos)
{
	// FIXME: optimize update area
	//update(0, 0, width(), height());
	update();
}

void TextView::insertChanged(bool)
{
	// FIXME: optimize update area
	// update curosr pos
	//update(0, 0, width(), height());
	update();
}


void TextView::selectionUpdate(quint64 begin, quint64 end)
{
	redrawSelection(begin, end);
}


}	// namespace
