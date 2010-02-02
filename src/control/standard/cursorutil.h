
#pragma once

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
	};
}
