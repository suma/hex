
#include "textview.h"
#include "caretdrawer.h"
#include "cursor.h"

namespace Standard {

CaretDrawer::CaretDrawer()
{
}


CaretDrawer::~CaretDrawer()
{
}


TextCaretDrawer::TextCaretDrawer(TextConfig &config)
	: config_(config)
{
}

void TextCaretDrawer::drawCaret(CaretDrawInfo info)
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



HexCaretDrawer::HexCaretDrawer(HexConfig &config, Cursor *cursor)
	: config_(config)
	, cursor_(cursor)
{
}

void HexCaretDrawer::drawCaret(CaretDrawInfo info)
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

void HexCaretDrawer::drawCaretLine(const CaretDrawInfo &info)
{
	int x;
	if (cursor_->nibble() || !info.caret_middle) {
		x = config_.x(info.x);
	} else {
		x = config_.x(info.x) + config_.byteMargin().left() + config_.charWidth();
	}
	QBrush brush(config_.color(Color::CaretBackground));
	info.painter.fillRect(x, info.y, 2, config_.byteHeight(), brush);
}

void HexCaretDrawer::drawCaretBlock(const CaretDrawInfo &info)
{
	QBrush brush(config_.color(Color::CaretBackground));
	ColorType color = cursor_->getSelection().color(info.pos);
	
	if (info.caret_middle) {
		if (cursor_->nibble() || cursor_->hasSelection()) {
			// Draw block byte
			info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
		} else {
			// Draw block lowwer nibble
			info.painter.fillRect(config_.x(info.x) + config_.byteMargin().left() + config_.charWidth(), info.y, config_.charWidth() + config_.byteMargin().right(), config_.byteHeight(), brush);
		}
	} else {
		// Draw block without data
		info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
	}
}

void HexCaretDrawer::drawCaretFrame(const CaretDrawInfo &info)
{
	int width, x;
	if (cursor_->nibble() || !info.caret_middle) {
		width = config_.byteWidth() - 1;
		x = config_.x(info.x);
	} else {
		width = config_.charWidth() + config_.byteMargin().right() - 1;
		x = config_.x(info.x) + config_.charWidth() + config_.byteMargin().left();
	}
	info.painter.setPen(config_.color(Color::CaretBackground));
	info.painter.drawRect(x, info.y, width, config_.byteHeight() - 1);
}

void HexCaretDrawer::drawCaretUnderbar(const CaretDrawInfo &info)
{
	int width, x;
	if (cursor_->nibble() || !info.caret_middle) {
		width = config_.byteWidth() - 1;
		x = config_.x(info.x);
	} else {
		width = config_.charWidth() + config_.byteMargin().right() - 1;
		x = config_.x(info.x) + config_.byteMargin().left() + config_.charWidth();
	}

	QBrush brush(config_.color(Color::CaretBackground));
	info.painter.fillRect(x, info.y + config_.byteHeight() - 2, width, 2, brush);
}




}	// namespace

