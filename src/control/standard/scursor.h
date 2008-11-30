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

	public:

		quint64 Position;
		bool Selected;

	};

}

#endif
