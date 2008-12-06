
#include "scursor.h"

namespace Standard {

Cursor::Cursor()
	: Top(0)
	, Position(0)
	, SelBegin(0)
	, SelEnd(0)
	, Selected(false)
	, Toggle(false)
{
}

void Cursor::refreshSelected()
{
	Selected = SelBegin != SelEnd;
}

}	// namespace
