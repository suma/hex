
#pragma once

#include <QtGlobal>
#include "../color.h"
#include "caret.h"

namespace Standard {

	struct HPos {
		quint64 X, Y;
	};

	struct CursorSelection {
		quint64 begin;
		quint64 end;
        quint64 anchor;

		bool selected(quint64 pos) const
		{
			return begin <= pos && pos < end;
		}

		ColorType color(quint64 pos, const Caret &caret) const
		{
			if (selected(pos)) {
				if (caret.visible() && pos == begin && pos != anchor) {
					return ColorType(Color::kCaretBackground, Color::kSelectText);
				} else {
					return ColorType(Color::kSelectBackground, Color::kSelectText);
				}
			} else {
				return ColorType(Color::kBackground, Color::kText);
			}
		}

	};
}
