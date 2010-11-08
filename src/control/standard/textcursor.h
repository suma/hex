
#pragma once

#include <QtGlobal>
#include "cursorutil.h"

class Document;

namespace Standard {

	class TextView;

	class TextCursor
	{
	public:
		TextCursor(Document *, TextView *);

	private:
		Document *document;
		TextView *view;

	public:

		quint64 Top;		// Number of Line
		quint64 Position;	// pos(not line)
		quint64 PositionAnchor;

		// Selected: [SelBegin, SelEnd]
		//bool Selected;		// Begin != End
		//bool Selection;
		bool Insert;
	
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

		CursorSelection getSelection() const;
	
	public:

		void movePosition(quint64 pos, bool sel, bool holdViewPos);
		void moveRelativePosition(qint64 pos, bool sel, bool holdViewPos);

	private:
		void redrawSelection(quint64, quint64);

	};

	inline void TextCursor::resetAnchor()
	{
		PositionAnchor = Position;
	}

	inline bool TextCursor::hasSelection()
	{
		return Position != PositionAnchor;
	}

	inline void TextCursor::reverseInsert()
	{
		Insert = !Insert;
	}

	inline CursorSelection TextCursor::getSelection() const
	{
		CursorSelection c;
		c.begin = qMin(Position, PositionAnchor);
		c.end = qMax(Position, PositionAnchor);
		return c;
	}
}


