
#include <QtGui>
#include "editor.h"
#include "control/standard.h"
#include "control/document.h"
#include "control/standard/caretdrawer.h"

Editor::Editor(QWidget *parent)
	: QWidget(parent)
	, document_(new Document())
{
	// test
#if 1
	srand(173);
	//for (int i = 0; i < 1064; i++) {
	//for (int i = 0; i < 10 * 16; i++) { rand(); }
	for (int i = 0; i < 157 * 200; i++) {
		uchar c = rand() & 0xFF;
		document_->insert(i, &c, 1);
	}
	uchar h = 0xFF;
	document_->insert(10, &h, 1);
	document_->insert(24, &h, 1);
	document_->insert(30, &h, 1);
	document_->insert(50, &h, 1);
#endif

	initView();
}

Editor::Editor(QWidget *parent, Document *document)
	: QWidget(parent)
	, document_(document)
{
	initView();
}

Document *Editor::document() const
{
	return document_;
}

void Editor::initView()
{
	setFocusPolicy(Qt::StrongFocus);

	// create address view
	view_ = new Standard::AddressView(this, document_);
	view_->move(0, 0);
	view_->resize(width(), height());
	view_->show();

	Standard::HexView *hex = new Standard::HexView(view_, document_);
	hex->show();

	Standard::TextView *text = new Standard::TextView(view_, document_);
	text->show();

	// sync cursor
	Standard::Cursor &hc = hex->cursor();
	Standard::Cursor &tc = text->cursor();
	hc.connectTo(&tc);
	tc.connectTo(&hc);

	// sync cursor to address view
	view_->connect(&hc);


	// set hex/text
	view_->setHexView(hex);
	view_->setTextView(text);

	// Caret
	Standard::CaretDrawer *hex_caret = hex->createCaretWidget();
	Standard::CaretDrawer *text_caret = text->createCaretWidget();


	view_->addHex(static_cast<QWidget*>(hex_caret));
	view_->addText(static_cast<QWidget*>(text_caret));

	connect(hex, SIGNAL(focusIn()), hex_caret, SLOT(enable()));
	connect(hex, SIGNAL(focusOut()), hex_caret, SLOT(disable()));
	connect(text, SIGNAL(focusIn()), text_caret, SLOT(enable()));
	connect(text, SIGNAL(focusOut()), text_caret, SLOT(disable()));

	hex->setFocus(Qt::OtherFocusReason);
}

void Editor::paintEvent(QPaintEvent *ev)
{
	// FIXME: set background
	QPainter painter(this);
	painter.fillRect(rect(), QBrush(QColor(200,200,255), Qt::CrossPattern));
}

void Editor::resizeEvent(QResizeEvent *)
{
	view_->resize(width(), height());
}

void Editor::focusInEvent(QFocusEvent *)
{
	view_->setFocus(Qt::OtherFocusReason);
}






