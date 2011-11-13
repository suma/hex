
#include "keyboard.h"
#include "../document.h"
#include "cursor.h"
#include "hexview.h"
#include "global.h"

namespace Standard {


Keyboard::Keyboard(Global *global, HexView *view)
	: ::KeyboardHandler(global->document())
	, global_(global)
	, view_(view)
{
}

Keyboard::~Keyboard()
{
}



void Keyboard::keyPressEvent(QKeyEvent *ev)
{
	bool keepAnchor = ev->modifiers() & Qt::SHIFT ? true : false;
	Cursor &cursor = global_->cursor();
	const int NUM = global_->config().num();

	switch (ev->key()) {
	case Qt::Key_Home:
		cursor.setNibble(true);
		view_->cursor().movePosition(global_, 0, keepAnchor, false);
		break;
	case Qt::Key_End:
		cursor.setNibble(true);
		view_->cursor().movePosition(global_, document_->length(), keepAnchor, false);
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
		view_->moveRelativePosition((qint64)-1 * NUM, keepAnchor, false);
		break;
	case Qt::Key_Down:
		cursor.setNibble(true);
		view_->moveRelativePosition((qint64)NUM, keepAnchor, false);
		break;
	case Qt::Key_PageUp:
		cursor.setNibble(true);
		view_->moveRelativePosition((qint64)-1 * NUM * 15, keepAnchor, true);
		break;
	case Qt::Key_PageDown:
		cursor.setNibble(true);
		view_->moveRelativePosition((qint64)NUM * 15, keepAnchor, true);
		break;
	case Qt::Key_Backspace:
		if (cursor.hasSelection()) {
			const quint64 pos = qMin(cursor.position(), cursor.anchor());
			const quint64 len = qMax(cursor.position(), cursor.anchor()) - pos;
			removeData(pos, len);
			view_->moveRelativePosition(pos, false, false);
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

	if (cursor.insert()) {
		// Inserte mode
		const CursorSelection selection = cursor.getSelection();
		const quint64 pos = selection.begin;

		// Replace data if selected
		if (cursor.hasSelection()) {
			// TODO: lazy redrawing for insertion
			removeData(pos, selection.end - pos);

			cursor.setPosition(pos);
			cursor.resetAnchor();
		}

		if (cursor.nibble()) {
			// insert
			insertData(pos, nibble << 4);
			cursor.setNibble(false);
		} else {
			if (pos + 1 > document_->length()) {
				return;
			}
			// replace
			uchar currentCharacter;
			document_->get(pos, &currentCharacter, 1);
			uchar c = nibble | (currentCharacter & 0xf0);
			cursor.inverseNibble();

			view_->moveRelativePosition(1, false, false);
			changeData(cursor.position() - 1, 1, &c, 1);
		}
	} else {
		// Ovewrite mode
		if (cursor.position() + 1 > document_->length()) {
			// no buffer
			return;
		}

		uchar currentCharacter;
		document_->get(cursor.position(), &currentCharacter, 1);
		if (cursor.nibble()) {
			uchar c = (nibble << 4) | (currentCharacter & 0x0f);
			changeData(cursor.position(), 1, &c, 1);
			cursor.setNibble(false);
		} else {
			uchar c = nibble | (currentCharacter & 0xf0);
			cursor.inverseNibble();

			view_->moveRelativePosition(1, false, false);
			changeData(cursor.position() - 1, 1, &c, 1);
		}
	}

}




}	// namespace
