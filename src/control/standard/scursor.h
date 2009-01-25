#ifndef STDCURSOR_H_INC
#define STDCURSOR_H_INC

#include <QtGlobal>

class Document;

namespace Standard {

	class HexView;

	struct HPos {
		quint64 X, Y;
	};

	class Cursor
	{
	public:
		Cursor(Document *, HexView *);
		void refreshSelected();
		bool selMoved();

	private:
		Document *document;
		HexView *view;

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
		void Left(uint = 0);
		void Right(uint = 0);
		void Up(uint = 0);
		void Down(uint = 0);
		void PageUp(uint = 0);
		void PageDown(uint = 0);


	};

}

#endif
