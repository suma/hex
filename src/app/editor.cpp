
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

void Editor::initView()
{
	Standard::HexView *hview_;
	Standard::TextView *tview_;


	aview_ = new Standard::AddressView(this, document_);
	aview_->move(0, 0);
	aview_->resize(width(), height());
	aview_->show();

	hview_ = new Standard::HexView(aview_, document_);
	hview_->setCaretBlink(true);
	hview_->add(hview_->createCaretWidget());

	hview_->show();

	tview_ = new Standard::TextView(aview_, document_);
	tview_->setCaretBlink(true);
	tview_->add(tview_->createCaretWidget());
	tview_->show();
	Standard::Cursor &hc = hview_->cursor();
	Standard::Cursor &tc = tview_->cursor();

	hc.connectTo(&tc);
	tc.connectTo(&hc);

	aview_->connect(&hc);	// hex cursor and addressview
	aview_->setHexView(hview_);
	aview_->setTextView(tview_);

	//setWindowOpacity(0.8);
}

void Editor::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	painter.fillRect(rect(), QBrush(QColor(200,200,255), Qt::CrossPattern));
}

void Editor::resizeEvent(QResizeEvent *resize)
{
	if (aview_) {
		aview_->resize(width(), height());
	}
}

