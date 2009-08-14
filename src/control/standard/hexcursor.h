
#pragma once

#include <QtGlobal>
#include "cursorutil.h"

class Document;

namespace Standard {

	class HexView;

	class HexCursor
	{
	public:
		HexCursor(Document *, HexView *);
		void refreshSelected();
		bool selMoved();

	private:
		Document *document;
		HexView *view;

	public:

		quint64 Top;		// Number of Line
		quint64 Position;	// pos(not line)
		quint64 PositionAnchor;

		// Selected: [SelBegin, SelEnd]
		//bool Selected;		// Begin != End
		//bool Selection;
		bool HighNibble;
		bool Insert;
	
		// for Caret
		CaretShape CaretVisibleShape;
		CaretShape CaretInvisibleShape;
		int CaretTimerId;
		bool CaretVisible;

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

		void resetAnchor();

		bool hasSelection();
		void setSelection(bool);

		void reverseInsert();

		void setCaretVisible(bool t);
		void turnCaretVisible();
	
	public:

		void movePosition(quint64 pos, bool sel, bool holdViewPos);
		void moveRelativePosition(qint64 pos, bool sel, bool holdViewPos);

	private:
		void redrawSelection(quint64, quint64);

	};

	inline void HexCursor::resetAnchor()
	{
		PositionAnchor = Position;
	}

	inline bool HexCursor::hasSelection()
	{
		return Position != PositionAnchor;
	}

	inline void HexCursor::reverseInsert()
	{
		Insert = !Insert;
	}

	inline void HexCursor::setCaretVisible(bool t)
	{
		CaretVisible = t;
	}

	inline void HexCursor::turnCaretVisible()
	{
		CaretVisible = !CaretVisible;
	}

}

