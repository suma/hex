
#include <algorithm>
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
	// 移動で選択操作をするとき
	// 1.古い位置のキャレットを消す描画 2.カーソル位置を更新する 3.選択範囲の行のみ再描画 4. キャレット描画　という流れになるようだ。選択じゃないときは少しプロセスが減る。
	// 移動だけのときは
	// 1.カーソル位置移動 2.古いカーソル位置の再描画（必要があれば） 3. キャレット再描画

	Q_ASSERT(pos <= document->length());
	
	const quint64 old_top = Top;


	if (!Selection) {
		if (sel) {
			// # Begin selection
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
			// normal move
			//   only redrawCaret
		}
	} else {
		if (sel) {
			// # Move selection
			// Set moved position to OLD
			SelEndOld = SelEnd;

			// Set moved position
			SelEnd = Position = pos;

			// Refresh flag
			refreshSelected();

			// Redraw updated lines
			view->drawSelected(false);

			//-- Redraw caret if caret selection moved
			if (view->getConfig().EnableCaret && SelEnd != SelEndOld) {
				view->drawCaret();
				setHexCaretVisible(false);
			}
		} else {
			// # End selection
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
	// if !selected
	// 	drawSelected(true)
	// 	selEnd = pos = newpos
	// 	redrawCaret
	// f refresh =>
	// 	selEnd= Pos = newPos
	// 	drawSelected(false);
	// 	drawCaret

	if (old_top == Top) {
		if (!sel && Selected) {
			// beginning selection
			//drawSelected(true);
		} else {
			//drawSelected(false);
		}
	}
}

void Cursor::moveRelativePosition(qint64 pos, bool sel, bool hold_vpos)
{
}

void Cursor::refreshTopByDown()
{
	const int count_line = view->getConfig().drawableLines(view->height()) - 1;
	const quint64 pos_line = Position / HexConfig::Num;

	// if Top + count_line < pos_line then Pos is invisible
	if (count_line <= pos_line && Top <= pos_line - count_line) {
		Top = pos_line - count_line + 1;
	}
}



}	// namespace
