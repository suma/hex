#ifndef STDCURSOR_H_INC
#define STDCURSOR_H_INC

#include <QtGlobal>

namespace Standard {

	struct HPos {
		quint64 X, Y;
	};

	class Cursor
	{
	public:
		Cursor();
		void refreshSelected();
		bool selMoved();

	public:

		quint64 Top;		// Number of Line
		quint64 Position;	// pos(not line)

		// Selected: [SelBegin, SelEnd]
		quint64 SelBegin;	// pos(not line)
		quint64 SelEnd;		// pos(not line)
		quint64 SelEndO;	// old
		bool Selected;		// Begin != End
		bool Toggle;

	};

}

#endif
