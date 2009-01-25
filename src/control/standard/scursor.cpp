
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
	qDebug("left");
	if (Position < count) {
		Home();
	} else {
		Position = Position - count;
		SelEnd = Position;
		Top = min(Top, Position / HexConfig::Num);
	}
}

void Cursor::Right(uint count)
{
	qDebug("right");
	Position = min(Position + count, document->length());
	SelEnd = Position;
	// TODO: Consider compute Top
}

void Cursor::Up(uint)
{
	// TODO: implement
}

void Cursor::Down(uint)
{
	// TODO: implement
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
