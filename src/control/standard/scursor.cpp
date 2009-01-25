
#include "scursor.h"
#include "../document.h"
#include "hexview.h"

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

void Cursor::Home()
{
	Top = 0;
	Position = SelBegin = SelEnd = 0;
	refreshSelected();
}

void Cursor::End()
{
	Position = SelBegin = SelEnd = document->length();
	refreshSelected();
}

void Cursor::Left(uint)
{
}

void Cursor::Right(uint)
{
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
