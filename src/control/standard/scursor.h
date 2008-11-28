#ifndef STDCURSOR_H_INC
#define STDCURSOR_H_INC

#include <QtGlobal>
#include "../cursor.h"

namespace Standard {

	struct HPos {
		quint64 X, Y;
	};

	class CCursor : public ::Cursor
	{
	public:
		CCursor();

	public:

		quint64 Position;

	};

}

#endif
