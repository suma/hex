
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
	, SelBegin(0)
	, SelEnd(0)
	, SelEndOld(0)
	, Selected(false)
	, Selection(false)
	, HighNibble(true)
	, CaretVisibleShape(CARET_BLOCK)
	, CaretInvisibleShape(CARET_FRAME)
	, CaretTimerId(0)
	, HexCaretVisible(true)
	, TextCaretVisible(true)
{
}

void Cursor::Home()
{
	Top = 0;
	Position = SelEnd = 0;
}

void Cursor::End()
{
	Position = SelEnd = document->length();
	refreshTopByDown();
}

void Cursor::Left(uint count)
{
	if (Position < count) {
		Home();
		return;
	}

	Position = Position - count;
	SelEnd = Position;

	refreshTopByUp();
}

void Cursor::Right(uint count)
{
	if (document->length() < count || document->length() - count < Position) {
		End();
		return;
	}

	Position += count;
	SelEnd = Position;

	refreshTopByDown();
}

void Cursor::Up(uint count)
{
	if (Position < count * HexConfig::Num) {
		Home();
		return;
	}

	Position -= count * HexConfig::Num;
	SelEnd = Position;

	refreshTopByUp();
}

void Cursor::Down(uint count)
{
	if (document->length() < count * HexConfig::Num || document->length() - count * HexConfig::Num < Position) {
		End();
		return;
	}

	Position += count * HexConfig::Num;
	SelEnd = Position;
	refreshTopByDown();
}

void Cursor::PageUp(uint)
{
	Up(view->getConfig().drawableLines(view->height()) / 2);
}

void Cursor::PageDown(uint)
{
	Down(view->getConfig().drawableLines(view->height()) / 2);
}

void Cursor::refreshTopByUp()
{
	Top = qMin(Position / HexConfig::Num, Top);
}

void Cursor::movePosition(quint64 pos, bool sel, bool hold_vpos)
{
	Q_ASSERT(pos <= document->length());
	// FIXME: replace drawView/drawCaret callings to udpate event
	
	if (!Selection) {
		if (sel) {
			//-- Begin selection --
			// Draw selected lines
			view->drawSelected(true);

			// Set begin position
			SelEndOld = Position;
			SelBegin = SelEnd = Position = pos;

			//-- Redraw lines if caret moved
			if (view->getConfig().EnableCaret && SelEnd != SelEndOld) {
				const int line = (SelEndOld / HexConfig::Num) - Top;
				if (line <= view->getConfig().drawableLines(view->height())) {
					view->drawView(HexView::DRAW_RANGE, line, line + 1);
				}
			}

			view->drawCaret();

			setSelection(true);
		} else {
			//-- Normal move --
			//   only redrawCaret
			quint64 top = Top;
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

			if (Top == top && Position != pos) {
				const int line = (Position / HexConfig::Num) - Top;
				view->drawView(HexView::DRAW_RANGE, line, line + 1);
			}

			SelEndOld = SelEnd = SelBegin = Position = pos;
			//qDebug("Top:%llu, top:%llu", Top, top);
			if (Top != top) {
				Top = top;
				view->drawView();
			}

			view->drawCaret();
		}
	} else {
		if (sel) {
			//-- Move selection
			// Set moved position to OLD
			SelEndOld = SelEnd;

			// Set moved position
			SelEnd = Position = pos;

			// Refresh flag
			refreshSelected();

			// Redraw updated lines
			view->drawSelected(false);

			//-- Redraw caret if caret selection moved --
			if (view->getConfig().EnableCaret && SelEnd != SelEndOld) {
				view->drawCaret();
				setHexCaretVisible(false);
			}
		} else {
			//-- End selection --
			// Set moved position
			SelEnd = Position = pos;
			refreshSelected();

			setSelection(false);

			// Redraw updated lines
			view->drawSelected(false);

			//-- Redraw caret if selection moved
			if (view->getConfig().EnableCaret && SelEnd != SelEndOld) {
				view->drawCaret();
				setHexCaretVisible(false);
			}
		}
	}
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

void Cursor::refreshTopByDown()
{
	const uint count_line = view->getConfig().drawableLines(view->height()) - 1;
	const quint64 pos_line = Position / HexConfig::Num;

	// if Top + count_line < pos_line then Pos is invisible
	if (count_line <= pos_line && Top <= pos_line - count_line) {
		Top = pos_line - count_line + 1;
	}
}



}	// namespace
