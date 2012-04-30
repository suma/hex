
#pragma once

#include "../color.h"

namespace Standard {

	struct HPos {
		quint64 X, Y;
	};

	enum CaretShape {
		CARET_NONE,
		CARET_LINE,
		CARET_BLOCK,
		CARET_FRAME,
		CARET_UNDERBAR,
	};

	struct CursorSelection {
		quint64 begin;
		quint64 end;

		bool selected(quint64 pos) const
		{
			return begin <= pos && pos < end;
		}

		ColorType color(quint64 pos) const
		{
			if (selected(pos)) {
				return ColorType(Color::kSelectBackground, Color::kSelectText);
			} else {
				return ColorType(Color::kBackground, Color::kText);
			}
		}
	};
}
