
#pragma once

#include <QWidget>
#include <QPainter>
#include "caret.h"

class Document;

namespace Standard {

	class Caret;
	class Cursor;
	class TextView;
	class TextConfig;
	class HexConfig;


	struct CaretDrawInfo
	{
		QPainter &painter;
		CaretShape shape;
		quint64 pos;
		bool caret_middle;
		int x;
		int y;

		CaretDrawInfo(QPainter &p, CaretShape shape, quint64 pos, int x, int y, bool caret_middle)
			: painter(p)
		{
			this->pos = pos;
			this->shape = shape;
			this->x = x;
			this->y = y;
			this->caret_middle = caret_middle;
		}
	};

	class CaretDrawer
	{
	public:
		CaretDrawer();
		virtual ~CaretDrawer();
		
		virtual void drawCaret(CaretDrawInfo info) = 0;
	};

	class TextCaretDrawer : public CaretDrawer, public QWidget
	{
	private:
		TextConfig &config_;
		Cursor *cursor_;
		::Document *document_;
		Caret caret_;
	public:
		TextCaretDrawer(TextConfig &config, Cursor *cursor, ::Document *document);
		void paintEvent(QPaintEvent*);
		void drawCaret(CaretDrawInfo info);

	private:
		void drawCaretLine(const CaretDrawInfo &info);
		void drawCaretBlock(const CaretDrawInfo &info);
		void drawCaretFrame(const CaretDrawInfo &info);
		void drawCaretUnderbar(const CaretDrawInfo &info);

	};

	class HexCaretDrawer : public CaretDrawer, public QWidget
	{
	private:
		HexConfig &config_;
		Cursor *cursor_;
		::Document *document_;
		Caret caret_;
	public:
		HexCaretDrawer(HexConfig &config, Cursor *cursor, ::Document *document);
		void paintEvent(QPaintEvent*);
		void drawCaret(CaretDrawInfo info);
	private:
		void drawCaretLine(const CaretDrawInfo &info);
		void drawCaretBlock(const CaretDrawInfo &info);
		void drawCaretFrame(const CaretDrawInfo &info);
		void drawCaretUnderbar(const CaretDrawInfo &info);
	};


}	// namespace


