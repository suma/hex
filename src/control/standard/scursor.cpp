
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
	// FIXME: replace drawView/drawCaret callings by doc udpate event
	
	const quint64 oldTop = Top;
	const quint64 oldPos = Position;
	const quint64 oldPosAnchor = PositionAnchor;
	const bool oldSelection = hasSelection();

	// Compute virtual position of caret
	int vwOldPosLine = 0;
	if (hold_vpos) {
		vwOldPosLine = Top - Position / HexConfig::Num;
	}

	const uint vwCountLine = view->getConfig().drawableLines(view->height()) - 1;

	//-- Update Cursor::Top with Position
	const bool goDown = Position < pos;
	if (goDown) {
		const quint64 posLine = pos / HexConfig::Num;

		// if Top + vwCountLine < posLine then Pos is invisible
		if (vwCountLine <= posLine && Top <= posLine - vwCountLine) {
			Top = posLine - vwCountLine + 1;
		}
	} else {
		Top = qMin(pos / HexConfig::Num, Top);
	}

	// Hold virtual position of caret
	if (hold_vpos) {
		const int vwNewPosLine = Top - pos / HexConfig::Num;
		const uint diff = qAbs(vwOldPosLine - vwNewPosLine);
		if (vwOldPosLine < vwNewPosLine) {
			if (diff < Top) {
				Top -= diff;
			} else {
				Top = 0;
			}
		} else {
			const quint64 maxTop = document->length() / HexConfig::Num - vwCountLine + 1;
			if (Top < numeric_limits<quint64>::max() - diff && Top + diff <= maxTop) {
				Top += diff;
			} else {
				Top = maxTop;
			}
		}
	}

	Position = pos;
	PositionAnchor = sel ? PositionAnchor : Position;

	// Reset Nibble
	//HighNibble = true;

	// Redraw view
	if (Top == oldTop) {
		if (!sel && oldSelection) {
			// Clear selection
			redrawSelection(qMin(oldPos, oldPosAnchor), qMax(oldPos, oldPosAnchor));
		} else if (Position != oldPos) {
			// Draw/Redraw selection
			const quint64 begin = qMin(qMin(Position, PositionAnchor), oldPos);
			const quint64 end   = qMax(qMax(Position, PositionAnchor), oldPos);
			redrawSelection(begin, end);
		}
		// Clear old caret
		view->drawView(HexView::DRAW_LINE, oldPos / HexConfig::Num - Top);
	} else {
		view->drawView();
	}

	view->drawCaret();
}


void Cursor::moveRelativePosition(qint64 pos, bool sel, bool hold_vpos)
{
	const quint64 diff = static_cast<quint64>(qAbs(pos));
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
	begin /= HexConfig::Num;
	end   /= HexConfig::Num;

	const int beginLine = qMax(begin, Top) - Top;
	const int endLine   = qMax(end, Top) - Top;

	//qDebug("redrawSelection %d, %d, Top:%llu", beginLine, endLine, Top);
	view->drawView(HexView::DRAW_RANGE, beginLine, endLine + 1);
}


}	// namespace
