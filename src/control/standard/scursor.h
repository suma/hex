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
		quint64 SelEndOld;	// old
		bool Selected;		// Begin != End
		bool Toggle;
		bool CaretHigh;

		int CaretTimerId;	// for Caret
		bool HexCaretVisible;
		bool StrCaretVisible;

		//QColor StrCaretColor;
	
	public:
		void Home();
		void End();
		void Left();
		void Right();
		void Up();
		void Down();
		void PageUp();
		void PageDown();


	};

}

#endif
