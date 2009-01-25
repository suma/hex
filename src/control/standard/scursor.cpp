
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
		Home();
	} else {
		Position = Position - count;
		SelEnd = Position;
		// Consider refresh Top
	}
}

void Cursor::Right(uint count)
{
	Position = min(Position + count, document->length());
	SelEnd = Position;
	// Consider refresh Top
}

void Cursor::Up(uint)
{
}

void Cursor::Down(uint)
{
}

void Cursor::PageUp(uint)
{
}

void Cursor::PageDown(uint)
{
}



}	// namespace
