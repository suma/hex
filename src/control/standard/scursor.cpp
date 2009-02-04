
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
}

void Cursor::Left(uint count)
{
	if (Position < count) {
		return;
	}

	Position = Position - count;
	SelEnd = Position;

	// TODO: refresh Top for up
}

void Cursor::Right(uint count)
{
	if (document->length() < count || document->length() - count < Position) {
		return;
	}

	Position += count;
	SelEnd = Position;

	// TODO: Consider compute Top
	int count_line = view->getConfig().drawableLines(view->height());

	// TODO: refresh Top for down
}

void Cursor::Up(uint count)
{
	if (Position < count * HexConfig::Num) {
		return;
	}

	Position -= count * HexConfig::Num;
	SelEnd = Position;
	// TODO: refresh Top for up
}

void Cursor::Down(uint count)
{
	if (document->length() < count * HexConfig::Num || document->length() - count * HexConfig::Num < Position) {
		return;
	}

	Position += count * HexConfig::Num;
	SelEnd = Position;
	// TODO: refresh Top for down
}

void Cursor::PageUp(uint)
{
	// TODO: implement
}

void Cursor::PageDown(uint)
{
	// TODO: implement
}



}	// namespace
