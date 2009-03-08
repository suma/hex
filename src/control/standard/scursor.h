#ifndef STDCURSOR_H_INC
#define STDCURSOR_H_INC

#include <QtGlobal>

class Document;

namespace Standard {

	class HexView;

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

	enum Move {
		OriginBegin,
		OriginEnd,
		OriginCurrent,
	};

	enum MoveMode {
		MoveAnchor,
		KeepAnchor,
	};

	class Cursor
	{
	public:
		Cursor(Document *, HexView *);
		void refreshSelected();
		bool selMoved();
		void resetSelection();

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
		bool HighNibble;
	
		// for Caret
		CaretShape CaretVisibleShape;
		CaretShape CaretInvisibleShape;
		int CaretTimerId;
		bool HexCaretVisible;
		bool TextCaretVisible;

		//QColor StrCaretColor;
	
	public:

		/*
		quint64 getSelectBegin();
		quint64 getSelectEnd();
		quint64 getSelectEndOld();
		void setSelectBegin(quint64);
		void setSelectEnd(quint64);
		void setSelectEndOld(quint64);
		*/

		void setHexCaretVisible(bool t);
		void turnHexCaretVisible();
		void setTextCaretVisible(bool t);
		void turnTextCaretVisible();

	public:
		void Home();
		void End();
		void Left(uint = 1);
		void Right(uint = 1);
		void Up(uint = 1);
		void Down(uint = 1);
		void PageUp(uint = 1);
		void PageDown(uint = 1);

		//void movePosition(qint64 diff, );

	private:
		void refreshTopByUp();
		void refreshTopByDown();

	};

	inline void Cursor::setHexCaretVisible(bool t)
	{
		HexCaretVisible = t;
	}

	inline void Cursor::turnHexCaretVisible()
	{
		HexCaretVisible = !HexCaretVisible;
	}

	inline void Cursor::setTextCaretVisible(bool t)
	{
		TextCaretVisible = t;
	}

	inline void Cursor::turnTextCaretVisible()
	{
		TextCaretVisible = !TextCaretVisible;
	}
}



#endif
