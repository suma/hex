
#include "scursor.h"

namespace Standard {

Cursor::Cursor()
	: Top(0)
	, Position(0)
	, SelBegin(0)
	, SelEnd(0)
	, SelEndO(0)
	, Selected(false)
	, Toggle(false)
	, CaretHigh(true)
	, HexTimerId(0)
	, StrTimerId(0)
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
	return SelEnd != SelEndO;
}

}	// namespace
