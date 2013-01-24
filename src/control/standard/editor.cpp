
#include <QtGui>
#include "editor.h"
#include "../standard.h"
#include "caretdrawer.h"

namespace Standard {

Editor::Editor(QWidget *parent)
	: QWidget(parent)
	, global_(new Global(this, new ::Document()))
{
	initView();
}

Editor::Editor(QWidget *parent, ::Document *document)
	: QWidget(parent)
	, global_(new Global(this, document))
{
	initView();
}

void Editor::initView()
{
	setFocusPolicy(Qt::StrongFocus);

	// create address view
	view_ = new AddressView(this, global_);
	view_->move(0, 0);
	view_->resize(width(), height());
	view_->show();

	HexView *hex = new HexView(view_, global_);
	hex->show();

	TextView *text = new TextView(view_, global_);
	text->show();

	// sync cursor
	Cursor &hc = hex->cursor();
	Cursor &tc = text->cursor();
	hc.connectTo(&tc);
	tc.connectTo(&hc);

	// sync cursor to address view
	view_->connect(&hc);


	// set hex/text
	view_->setHexView(hex);
	view_->setTextView(text);

	// Caret
	CaretDrawer *hex_caret = hex->createCaretWidget();
	CaretDrawer *text_caret = text->createCaretWidget();


	//view_->addHex(static_cast<QWidget*>(hex_caret));
	//view_->addText(static_cast<QWidget*>(text_caret));

	connect(hex, SIGNAL(focusIn()), hex_caret, SLOT(enable()));
	connect(hex, SIGNAL(focusOut()), hex_caret, SLOT(disable()));
	connect(text, SIGNAL(focusIn()), text_caret, SLOT(enable()));
	connect(text, SIGNAL(focusOut()), text_caret, SLOT(disable()));

	hex->setFocus(Qt::OtherFocusReason);
}

void Editor::paintEvent(QPaintEvent *ev)
{
	// FIXME: set background
	//QPainter painter(this);
	//painter.fillRect(rect(), QBrush(QColor(200,200,255), Qt::CrossPattern));
}

void Editor::resizeEvent(QResizeEvent *)
{
	view_->resize(width(), height());
}

void Editor::focusInEvent(QFocusEvent *)
{
	view_->setFocus(Qt::OtherFocusReason);
}

}	// namespace Standard
