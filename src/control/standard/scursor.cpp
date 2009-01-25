
#include "scursor.h"

namespace Standard {

Cursor::Cursor()
	: Top(0)
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
}

void Cursor::End()
{
}

void Cursor::Left()
{
}

void Cursor::Right()
{
}

void Cursor::Up()
{
}

void Cursor::Down()
{
}

void Cursor::PageUp()
{
}

void Cursor::PageDown()
{
}



}	// namespace
