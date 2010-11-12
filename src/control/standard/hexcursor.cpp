
#include <QtGlobal>
#include <algorithm>
#include <limits>
#include "hexcursor.h"
#include "../document.h"
#include "hexview.h"

using namespace std;

namespace Standard {

HexCursor::HexCursor(Document *Doc, HexView *View)
	: document(Doc)
	, view(View)
	, Top(0)
	, Position(0)
	, PositionAnchor(0)
	, HighNibble(true)
	, Insert(true)
{
}

void HexCursor::movePosition(quint64 pos, bool sel, bool holdViewPos)
{
	Q_ASSERT(pos <= document->length());
	// FIXME: replace drawView/drawCaret callings by doc udpate event
	
	// Compute virtual position of caret
	int vwOldPosLine = 0;
	if (holdViewPos) {
		vwOldPosLine = Top - Position / view->getConfig().getNum();	// FIXME
	}

	const uint vwCountLine = view->getConfig().drawableLines(view->height()) - 1;

	//-- Update Cursor::Top with Position
	const bool goDown = Position < pos;
	if (goDown) {
		const quint64 posLine = pos / view->getConfig().getNum();

		// if Top + vwCountLine < posLine then Pos is invisible
		if (vwCountLine <= posLine && Top <= posLine - vwCountLine) {
			Top = posLine - vwCountLine + 1;
		}
	} else {
		Top = qMin(pos / view->getConfig().getNum(), Top);
	}

	// Hold virtual position of caret
	if (holdViewPos) {
		const int vwNewPosLine = Top - pos / view->getConfig().getNum();
		const uint diff = qAbs(vwOldPosLine - vwNewPosLine);
		if (vwOldPosLine < vwNewPosLine) {
			if (diff < Top) {
				Top -= diff;
			} else {
				Top = 0;
			}
		} else {
			const quint64 maxTop = document->length() / view->getConfig().getNum() - vwCountLine + 1;
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
}


quint64 HexCursor::getRelativePosition(qint64 pos)
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
	
	return okPos;
}

void HexCursor::redrawSelection(quint64 begin, quint64 end)
{
	//qDebug("redrawSelection %llu, %llu, Top:%llu", begin, end, Top);
	begin /= view->getConfig().getNum();
	end   /= view->getConfig().getNum();

	const int beginLine = qMax(begin, Top) - Top;
	const int endLine   = qMax(end, Top) - Top;

	//qDebug("redrawSelection %d, %d, Top:%llu", beginLine, endLine, Top);
	view->drawView(DRAW_RANGE, beginLine, endLine + 1);
}


}	// namespace
