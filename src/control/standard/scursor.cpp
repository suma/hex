
#include <QtGlobal>
#include <algorithm>
#include <limits>
#include "scursor.h"
#include "../document.h"
#include "hexview.h"

using namespace std;

namespace Standard {

Cursor::Cursor(Document *Doc, HexView *View)
	: document(Doc)
	, view(View)
	, Top(0)
	, Position(0)
	, PositionAnchor(0)
	, HighNibble(true)
	, CaretVisibleShape(CARET_BLOCK)
	, CaretInvisibleShape(CARET_FRAME)
	, CaretTimerId(0)
	, HexCaretVisible(true)
	, TextCaretVisible(true)
{
}

void Cursor::movePosition(quint64 pos, bool sel, bool hold_vpos)
{
	Q_ASSERT(pos <= document->length());
	// FIXME: replace drawView/drawCaret callings to udpate event
	
	const quint64 oldPos = Position;
	quint64 top = Top;
	int vpos_line = 0;
	if (hold_vpos) {
		vpos_line = Top - Position / HexConfig::Num;
	}

	const bool goDown = Position < pos;
	bool hasSelection = Position != PositionAnchor;

	if (goDown) {
		const uint count_line = view->getConfig().drawableLines(view->height()) - 1;
		const quint64 pos_line = pos / HexConfig::Num;

		// if Top + count_line < pos_line then Pos is invisible
		if (count_line <= pos_line && Top <= pos_line - count_line) {
			top = pos_line - count_line + 1;
		}
	} else {
		top = qMin(pos / HexConfig::Num, Top);
	}

	if (hold_vpos) {
		const int vpos_line_now = top - pos / HexConfig::Num;
		const uint diff = qAbs(vpos_line - vpos_line_now);
		if (vpos_line < vpos_line_now) {
			if (diff < top) {
				top -= diff;
			} else {
				top = 0;
			}
		} else {
			const uint count_line = view->getConfig().drawableLines(view->height()) - 1;
			const quint64 max_top = document->length() / HexConfig::Num - count_line;
			if (top < numeric_limits<quint64>::max() - diff && top + diff <= max_top) {
				top += diff;
			} else {
				top = max_top;
			}
		}
	}

	if (!sel) {
		PositionAnchor = pos;
	}
	Position = pos;
	if (Top == top) {
	   if ((Position != oldPos || hasSelection)) {
			const int line_old_pos = (oldPos / HexConfig::Num) - Top;
			const int line_anchor = (PositionAnchor / HexConfig::Num) - Top;
			view->drawView(HexView::DRAW_RANGE, qMin(line_old_pos, line_anchor), qMax(line_old_pos, line_anchor) + 1);
	   }
	} else {
		Top = top;
		view->drawView();
	}

	view->drawCaret();
}


void Cursor::moveRelativePosition(qint64 pos, bool sel, bool hold_vpos)
{
	quint64 abs = static_cast<quint64>(qAbs(pos));
	quint64 okPos = 0;
	if (pos < 0) {
		if (Position < abs) {
			okPos = 0;
		} else {
			okPos = Position - abs;
		}
	} else {
		if (Position < numeric_limits<quint64>::max() - abs && Position + abs <= document->length()) {
			okPos = Position + abs;
		} else {
			okPos = document->length();
		}
	}
	//qDebug("pos:%lld, abs:%llu, okPos: %llu", pos, abs, okPos);
	movePosition(okPos, sel, hold_vpos);
}



}	// namespace
