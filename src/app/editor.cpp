
#include <QtGui>
#include "editor.h"
#include "control/standard.h"
#include "control/document.h"

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
	// create address view
	view_ = new Standard::AddressView(this, document_);
	view_->move(0, 0);
	view_->resize(width(), height());
	view_->show();

	Standard::HexView *hex = new Standard::HexView(view_, document_);
	hex->setCaretBlink(true);
	hex->add(hex->createCaretWidget());
	hex->show();

	Standard::TextView *text = new Standard::TextView(view_, document_);
	text->setCaretBlink(true);
	text->add(text->createCaretWidget());
	text->show();

	// sync cursor
	Standard::Cursor &hc = hex->cursor();
	Standard::Cursor &tc = text->cursor();
	hc.connectTo(&tc);
	tc.connectTo(&hc);

	// sync cursor to address view
	view_->connect(&hc);

	view_->setHexView(hex);
	view_->setTextView(text);

	//setWindowOpacity(0.8);
}

void Editor::paintEvent(QPaintEvent *ev)
{
	// FIXME: set background
	QPainter painter(this);
	painter.fillRect(rect(), QBrush(QColor(200,200,255), Qt::CrossPattern));
}

void Editor::resizeEvent(QResizeEvent *resize)
{
	view_->resize(width(), height());
}

