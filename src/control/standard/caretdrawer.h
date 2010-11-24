
#pragma once

#include <QWidget>
#include <QPainter>
#include "caret.h"

namespace Standard {

class TextView;
class TextConfig;

class CaretDrawer
{
private:
	TextView *view_;
	TextConfig &config_;

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
	CaretDrawer(TextView *view);
	~CaretDrawer();
	


	void drawCaret(QPainter *painter, int x, int y, bool visible, bool caret_middle);

	void drawCaretShape(CaretDrawInfo info);
	void drawCaretLine(const CaretDrawInfo &info);

	void drawCaretBlock(const CaretDrawInfo &info);
	void drawCaretFrame(const CaretDrawInfo &info);
	void drawCaretUnderbar(const CaretDrawInfo &info);


};


}	// namespace


