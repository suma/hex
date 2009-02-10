
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
	, Toggle(false)
	, CaretHigh(true)
	, CaretShape(CARET_FRAME)
	, CaretTimerId(0)
	, HexCaretVisible(true)
	, StrCaretVisible(true)
{
}

void Cursor::refreshSelected()
{
	Selected = SelBegin != SelEnd;
}

bool Cursor::selMoved()
{
	return SelEnd != SelEndOld;
}

void Cursor::resetSelection()
{
	SelBegin = SelEnd;
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
	const quint64 pos_line = Position / HexConfig::Num;

	if (pos_line < Top) {
		Top = pos_line;
	}
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
