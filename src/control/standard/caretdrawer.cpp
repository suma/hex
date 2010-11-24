
#include "textview.h"
#include "caretdrawer.h"

namespace Standard {



CaretDrawer::CaretDrawer(TextView *view)
	: view_(view)
	, config_(view->config())
{
}


CaretDrawer::~CaretDrawer()
{
}


void CaretDrawer::drawCaret(QPainter *painter, int x, int y, bool visible, bool caret_middle)
{
	// TODO: painter.setFont(config_.font());

	const CaretShape shape = view_->caret().getShape(visible);

	// Draw shape
	drawCaretShape(CaretDrawInfo(*painter, shape, x, y, caret_middle));
}


void CaretDrawer::drawCaretShape(CaretDrawInfo info)
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

void CaretDrawer::drawCaretLine(const CaretDrawInfo &info)
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

void CaretDrawer::drawCaretBlock(const CaretDrawInfo &info)
{
	if (info.caret_middle) {
		QBrush brush(config_.color(Color::CaretBackground));
		info.painter.setBackground(brush);
		info.painter.setPen(config_.color(Color::CaretText));
		info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
		// TODO: 本当はここで文字描画
	} else {
		// Draw block without data
		QBrush brush(config_.color(Color::CaretBackground));
		info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
	}
}

void CaretDrawer::drawCaretFrame(const CaretDrawInfo &info)
{
	int width = config_.byteWidth() - 1;
	int x = config_.x(info.x);

	info.painter.setPen(config_.color(Color::CaretBackground));
	info.painter.drawRect(x, info.y, width, config_.byteHeight() - 1);
}

void CaretDrawer::drawCaretUnderbar(const CaretDrawInfo &info)
{
	int width = config_.byteWidth() - 1;
	int x = config_.x(info.x);

	QBrush brush(config_.color(Color::CaretBackground));
	info.painter.fillRect(x, info.y + config_.byteHeight() - 2, width, 2, brush);
}









}	// namespace

