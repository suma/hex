
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

	Position = pos;
	PositionAnchor = sel ? PositionAnchor : Position;

	if (Top == oldTop) {
		if (!sel && oldSelection) {
			//qDebug("A");
			redrawSelection(qMin(oldPos, oldPosAnchor), qMax(oldPos, oldPosAnchor));
		} else if (oldSelection && Position != oldPos) {
			quint64 begin, end;
			if (PositionAnchor < oldPos && PositionAnchor >= Position ||
				PositionAnchor >=oldPos && PositionAnchor < Position) {
				// Crossing between begin and end
				begin = qMin(qMin(Position, PositionAnchor), oldPos);
				end   = qMax(qMax(Position, PositionAnchor), oldPos);
				//qDebug("B - crossing %llu, %llu", begin/16, end/16);
			} else {
				// Minimum range
				if (Position < oldPos) {
					begin = qMin(Position, oldPos);
					end   = qMax(qMax(Position, oldPos), PositionAnchor);
				} else {
					begin = qMin(qMin(Position, oldPos), PositionAnchor);
					end   = qMax(Position, oldPos);
				}
				//qDebug("C - Pos:%llu, oldPos:%llu Anch:%llu oldAnch:%llu", Position/16, oldPos/16, PositionAnchor/15, oldPosAnchor/16);
				//qDebug("C - minimum %llu, %llu", begin, end);
			}
			redrawSelection(begin, end);
		}
		// 
		view->drawView(HexView::DRAW_LINE, oldPos / HexConfig::Num - Top);
	} else {
		view->drawView();
	}

	view->drawCaret();
}


void Cursor::moveRelativePosition(qint64 pos, bool sel, bool hold_vpos)
{
	quint64 diff = static_cast<quint64>(qAbs(pos));
	quint64 okPos = 0;
	if (pos < 0) {
		if (Position < diff) {
			okPos = 0;
		} else {
			okPos = Position - diff;
		}
	} else {
		if (Position < numeric_limits<quint64>::max() - diff && Position + diff <= document->length()) {
			okPos = Position + diff;
		} else {
			okPos = document->length();
		}
	}
	//qDebug("pos:%lld, diff:%llu, okPos: %llu", pos, diff, okPos);
	movePosition(okPos, sel, hold_vpos);
}

void Cursor::redrawSelection(quint64 begin, quint64 end)
{
	//qDebug("redrawSelection %llu, %llu, Top:%llu", begin, end, Top);
	int beginLine, endLine;
	begin /= HexConfig::Num;
	end   /= HexConfig::Num;

	beginLine = qMax(begin, Top) - Top;
	endLine   = qMax(end, Top) - Top;

	//qDebug("redrawSelection %d, %d, Top:%llu", beginLine, endLine, Top);
	view->drawView(HexView::DRAW_RANGE, beginLine, endLine + 1);
}


}	// namespace
