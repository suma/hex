
#include "textview.h"
#include "caretdrawer.h"

namespace Standard {

CaretDrawer::CaretDrawer(const Caret &caret)
	: caret_(caret)
{
}


CaretDrawer::~CaretDrawer()
{
}


void CaretDrawer::drawCaret(QPainter *painter, int x, int y, bool visible, bool caret_middle)
{
	CaretDrawInfo info(*painter, caret_.shape(visible), x, y, caret_middle);
	drawCaretShape(info);
}

TextCaretDrawer::TextCaretDrawer(const Caret &caret, TextConfig &config)
	: CaretDrawer(caret)
	, config_(config)
{
}

void TextCaretDrawer::drawCaretShape(CaretDrawInfo info)
{
	switch (info.shape) {
	case CARET_LINE:
		drawCaretLine(info);
		break;
	case CARET_BLOCK:
		drawCaretBlock(info);
		break;
	case CARET_FRAME:
		drawCaretFrame(info);
		break;
	case CARET_UNDERBAR:
		drawCaretUnderbar(info);
		break;
	default:
		;
	}
}

// FIXME:  caret width(for Multibyte Character)

void TextCaretDrawer::drawCaretLine(const CaretDrawInfo &info)
{
	int x;
	if (true) {//if (!info.caret_middle) {
		x = config_.x(info.x);
	} else {
		x = config_.x(info.x) + config_.charWidth();
	}
	QBrush brush(config_.color(Color::CaretBackground));
	info.painter.fillRect(x, info.y, 2, config_.byteHeight(), brush);
}

void TextCaretDrawer::drawCaretBlock(const CaretDrawInfo &info)
{
	QBrush brush(config_.color(Color::CaretBackground));
	if (info.caret_middle) {
		info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
		// TODO: 本当はここで文字描画
	} else {
		// Draw block without data
		info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
	}
}

void TextCaretDrawer::drawCaretFrame(const CaretDrawInfo &info)
{
	int width = config_.byteWidth() - 1;
	int x = config_.x(info.x);

	info.painter.setPen(config_.color(Color::CaretBackground));
	info.painter.drawRect(x, info.y, width, config_.byteHeight() - 1);
}

void TextCaretDrawer::drawCaretUnderbar(const CaretDrawInfo &info)
{
	int width = config_.byteWidth() - 1;
	int x = config_.x(info.x);

	QBrush brush(config_.color(Color::CaretBackground));
	info.painter.fillRect(x, info.y + config_.byteHeight() - 2, width, 2, brush);
}



HexCaretDrawer::HexCaretDrawer(const Caret &caret, HexConfig &config)
	: CaretDrawer(caret)
	, config_(config)
{
}

void HexCaretDrawer::drawCaretShape(CaretDrawInfo info)
{
}

void HexCaretDrawer::drawCaretLine(const CaretDrawInfo &info)
{
}

void HexCaretDrawer::drawCaretBlock(const CaretDrawInfo &info)
{
}

void HexCaretDrawer::drawCaretFrame(const CaretDrawInfo &info)
{
}

void HexCaretDrawer::drawCaretUnderbar(const CaretDrawInfo &info)
{
}




}	// namespace

