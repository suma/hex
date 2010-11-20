
#include "keyboard.h"
#include "keyboard.h"
#include "../document.h"
#include "cursor.h"
#include "hexview.h"

namespace Standard {


Keyboard::Keyboard(::Document *doc, HexView *view)
	: ::KeyboardHandler(doc)
	, view_(view)
{
}

Keyboard::~Keyboard()
{
}



void Keyboard::keyPressEvent(QKeyEvent *ev)
{
	bool keepAnchor = ev->modifiers() & Qt::SHIFT ? true : false;
	Cursor &cursor = view_->cursor();
	HexConfig &config = view_->config();

	switch (ev->key()) {
	case Qt::Key_Home:
		cursor.setNibble(true);
		view_->movePosition(0, keepAnchor, false);
		break;
	case Qt::Key_End:
		cursor.setNibble(true);
		view_->movePosition(document_->length(), keepAnchor, false);
		break;
	case Qt::Key_Left:
		cursor.setNibble(true);
		view_->moveRelativePosition(-1, keepAnchor, false);
		break;
	case Qt::Key_Right:
		cursor.setNibble(true);
		view_->moveRelativePosition(1, keepAnchor, false);
		break;
	case Qt::Key_Up:
		cursor.setNibble(true);
		view_->moveRelativePosition((qint64)-1 * config.getNum(), keepAnchor, false);
		break;
	case Qt::Key_Down:
		cursor.setNibble(true);
		view_->moveRelativePosition((qint64)config.getNum(), keepAnchor, false);
		break;
	case Qt::Key_PageUp:
		cursor.setNibble(true);
		view_->moveRelativePosition((qint64)-1 * config.getNum() * 15, keepAnchor, true);
		break;
	case Qt::Key_PageDown:
		cursor.setNibble(true);
		view_->moveRelativePosition((qint64)config.getNum() * 15, keepAnchor, true);
		break;
	case Qt::Key_Backspace:
		if (cursor.hasSelection()) {
			const quint64 pos = qMin(cursor.position(), cursor.anchor());
			const quint64 len = qMax(cursor.position(), cursor.anchor()) - pos;
			removeData(pos, len);
			view_->moveRelativePosition(pos, false, false);
			// TODO: drawView [pos. pos+len]
			//drawView();
			cursor.setNibble(true);
		} else if (0 < cursor.position()) {
			removeData(cursor.position() - 1, 1);
			view_->moveRelativePosition(-1, false, false);
			cursor.setNibble(true);
		}
		break;
	case Qt::Key_Insert:
		qDebug("key insert");
		cursor.reverseInsert();
		break;
	case Qt::Key_Delete:
		if (cursor.hasSelection()) {
			const quint64 pos = qMin(cursor.position(), cursor.anchor());
			const quint64 len = qMax(cursor.position(), cursor.anchor()) - pos;
			removeData(pos, len);
			view_->moveRelativePosition(0, false, false);
			// TODO: drawView [pos. pos+len]
			//drawView();
			cursor.setNibble(true);
		} else if (cursor.position() < document_->length()) {
			removeData(cursor.position(), 1);
			view_->moveRelativePosition(0, false, false);
			cursor.setNibble(true);
		}
		break;
	default:
		{
			QString text = ev->text();
			qDebug() << "hoge";
			for (int i = 0; i < text.length(); i++) {
				keyInputEvent(text.at(i));
			}
		}
	}
}

void Keyboard::keyInputEvent(QString str)
{
}


void Keyboard::keyInputEvent(QChar ch)
{
	// if hexview
	//
	//

	ch = ch.toLower();
	int nibble = -1;
	if (ch.unicode() >= 'a' && ch.unicode() <= 'f') {
		nibble = ch.unicode() - 'a' + 10;
	} else if (ch.unicode() >= '0' && ch.unicode() <= '9') {
		nibble = ch.unicode() - '0';
	}
	if (nibble < 0) {
		return;
	}

	// match view
	Cursor &cursor = view_->cursor();

	//if (cursor.insert()) {
	if (false) {
		// Inserte mode
		quint64 pos = qMin(cursor.position(), cursor.anchor());

		// Replace data if selected
		if (cursor.hasSelection()) {
			// Off redrawing temporary for redrawing on insertion
			removeData(pos, qMax(cursor.position(), cursor.anchor()) - pos);

			cursor.setPosition(pos);
			cursor.resetAnchor();
		}

		insertData(pos, nibble << 4);
		cursor.setNibble(false);

		// TODO: ::Keyboard
		// moveRelativePosition(1, ..);
	} else {
		// Ovewrite mode
		uchar currentCharacter;
		document_->get(cursor.position(), &currentCharacter, 1);
		if (cursor.nibble()) {
			uchar c = (nibble << 4) | (currentCharacter & 0x0f);
			changeData(cursor.position(), 1, &c, 1);
			cursor.setNibble(false);

			// FIXME: redraw caret
			view_->drawCaret();
		} else {
			// TODO: ::Keyboard
			//moveRelativePosition(1, ...);
			uchar c = nibble | (currentCharacter & 0xf0);
			cursor.inverseNibble();

			view_->moveRelativePosition(1, false, false);
			changeData(cursor.position() - 1, 1, &c, 1);
		}
	}

}




}	// namespace
