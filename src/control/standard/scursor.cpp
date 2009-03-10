
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
	
	const quint64 oldTop = Top;
	const quint64 oldPos = Position;
	const quint64 oldPosAnchor = PositionAnchor;
	const bool oldSelection = hasSelection();

	int vpos_line = 0;
	if (hold_vpos) {
		vpos_line = Top - Position / HexConfig::Num;
	}

	const bool goDown = Position < pos;
	if (goDown) {
		const uint count_line = view->getConfig().drawableLines(view->height()) - 1;
		const quint64 pos_line = pos / HexConfig::Num;

		// if Top + count_line < pos_line then Pos is invisible
		if (count_line <= pos_line && Top <= pos_line - count_line) {
			Top = pos_line - count_line + 1;
		}
	} else {
		Top = qMin(pos / HexConfig::Num, Top);
	}

	if (hold_vpos) {
		const int vpos_line_now = Top - pos / HexConfig::Num;
		const uint diff = qAbs(vpos_line - vpos_line_now);
		if (vpos_line < vpos_line_now) {
			if (diff < Top) {
				Top -= diff;
			} else {
				Top = 0;
			}
		} else {
			const uint count_line = view->getConfig().drawableLines(view->height()) - 1;
			const quint64 max_top = document->length() / HexConfig::Num - count_line;
			if (Top < numeric_limits<quint64>::max() - diff && Top + diff <= max_top) {
				Top += diff;
			} else {
				Top = max_top;
			}
		}
	}

	if (!sel) {
		PositionAnchor = pos;
	}
	Position = pos;

	if (Top == oldTop) {
		int line_pos, line_anchor;
		if (Position != oldPos) {
			view->drawView(HexView::DRAW_LINE, oldPos / HexConfig::Num - oldTop);
		}
		if (oldSelection) {
			redrawSelection(qMin(oldPos, oldPosAnchor), qMax(oldPos, oldPosAnchor), oldTop);
		}
		if (hasSelection()) {
			redrawSelection(qMin(Position, PositionAnchor), qMax(Position, PositionAnchor), Top);
		}
	} else {
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

void Cursor::redrawSelection(quint64 begin, quint64 end, quint64 top)
{
	qDebug("redrawSelection %llu, %llu, Top:%llu", begin, end, top);
	int beginLine, endLine;
	begin /= HexConfig::Num;
	end   /= HexConfig::Num;

	Q_ASSERT(end > top);

	beginLine = qMax(begin, top) - top;
	endLine   = end - top;

	view->drawView(HexView::DRAW_RANGE, beginLine, endLine + 1);
}


}	// namespace
