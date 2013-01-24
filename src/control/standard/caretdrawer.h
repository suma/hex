
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

	class CaretDrawer : public QObject
	{
		Q_OBJECT

	public:
		CaretDrawer(QWidget *parent = NULL);
		virtual ~CaretDrawer();
		
		virtual void drawCaret(CaretDrawInfo info) = 0;

		void setCaretBlink(bool enable);
	public slots:
		void enable();
		void disable();

	protected:
		void timerEvent(QTimerEvent *);

	protected:
		Caret caret_;
	};

	class TextCaretDrawer : public CaretDrawer
	{
		Q_OBJECT
	private:
		TextConfig &config_;
		Cursor *cursor_;
		::Document *document_;
	public:
		TextCaretDrawer(QWidget *parent, TextConfig &config, Cursor *cursor, ::Document *document);
		~TextCaretDrawer();
		void paintEvent(QPaintEvent*);
		void drawCaret(CaretDrawInfo info);

	private:
		void drawCaretLine(const CaretDrawInfo &info);
		void drawCaretBlock(const CaretDrawInfo &info);
		void drawCaretFrame(const CaretDrawInfo &info);
		void drawCaretUnderbar(const CaretDrawInfo &info);

	};

	class HexCaretDrawer : public CaretDrawer
	{
		Q_OBJECT
	private:
		HexConfig &config_;
		Cursor *cursor_;
		::Document *document_;
	public:
		HexCaretDrawer(QWidget *parent, HexConfig &config, Cursor *cursor, ::Document *document);
		~HexCaretDrawer();
		void paintEvent(QPaintEvent*);
		void drawCaret(CaretDrawInfo info);
	private:
		void drawCaretLine(const CaretDrawInfo &info);
		void drawCaretBlock(const CaretDrawInfo &info);
		void drawCaretFrame(const CaretDrawInfo &info);
		void drawCaretUnderbar(const CaretDrawInfo &info);
	};


}	// namespace


