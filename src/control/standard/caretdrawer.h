
#pragma once

#include <QWidget>
#include <QPainter>
#include "caret.h"

namespace Standard {

class Caret;
class TextView;
class TextConfig;
class HexConfig;

class CaretDrawer
{
protected:
	const Caret &caret_;

	struct CaretDrawInfo
	{
		QPainter &painter;
		CaretShape shape;
		bool caret_middle;
		int x;
		int y;

		CaretDrawInfo(QPainter &p, CaretShape shape, int x, int y, bool caret_middle)
			: painter(p)
		{
			this->shape = shape;
			this->x = x;
			this->y = y;
			this->caret_middle = caret_middle;
		}
	};

public:
	CaretDrawer(const Caret &caret);
	virtual ~CaretDrawer();
	
	void drawCaret(QPainter *painter, int x, int y, bool visible, bool caret_middle);
	virtual void drawCaretShape(CaretDrawInfo info) = 0;
};

class TextCaretDrawer : public CaretDrawer
{
private:
	TextConfig &config_;
public:
	TextCaretDrawer(const Caret &caret, TextConfig &config);
	void drawCaretShape(CaretDrawInfo info);

private:
	void drawCaretLine(const CaretDrawInfo &info);
	void drawCaretBlock(const CaretDrawInfo &info);
	void drawCaretFrame(const CaretDrawInfo &info);
	void drawCaretUnderbar(const CaretDrawInfo &info);

};

class HexCaretDrawer : public CaretDrawer
{
private:
	HexConfig &config_;
public:
	HexCaretDrawer(const Caret &caret, HexConfig &config);
	void drawCaretShape(CaretDrawInfo info);
private:
	void drawCaretLine(const CaretDrawInfo &info);
	void drawCaretBlock(const CaretDrawInfo &info);
	void drawCaretFrame(const CaretDrawInfo &info);
	void drawCaretUnderbar(const CaretDrawInfo &info);
};


}	// namespace


