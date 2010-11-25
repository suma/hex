
#include <QtGui>
#include "editor.h"
#include "control/standard.h"
#include "control/document.h"

Editor::Editor()
{
	doc_= new Document();

	srand(173);
#if 1
	//for (int i = 0; i < 1064; i++) {
	//for (int i = 0; i < 10 * 16; i++) { rand(); }
	for (int i = 0; i < 157 * 200; i++) {
		uchar c = rand() & 0xFF;
		doc_->insert(i, &c, 1);
	}
	uchar h = 0xFF;
	doc_->insert(10, &h, 1);
	doc_->insert(24, &h, 1);
	doc_->insert(30, &h, 1);
	doc_->insert(50, &h, 1);
#endif


	hview_ = new Standard::HexView(this, doc_);
	hview_->setCaretBlink(true);

	
	aview_ = new Standard::AddressView(this, doc_, hview_);
	aview_->move(0, 0);
	aview_->resize(100, 270);
	aview_->show();

	resize(900,400);
	hview_->move(aview_->width(), 0);
	hview_->resize(hview_->config().width(), 270);
	qDebug("%d ", hview_->config().width());
	hview_->show();

	tview_ = new Standard::TextView(this, doc_);
	tview_->setCaretBlink(true);
	tview_->move(aview_->width() + hview_->config().width() + 20, 0);
	tview_->resize(tview_->config().width(), 270);
	tview_->show();

	Standard::Cursor &hc = hview_->cursor();
	Standard::Cursor &tc = tview_->cursor();
	hc.connectTo(&tc);
	tc.connectTo(&hc);


	// window transparency
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
		aview_->resize(aview_->width(), height());
	}
	if (hview_) {
		hview_->resize(hview_->width(), height());
	}
	if (tview_) {
		tview_->resize(tview_->config().width(), height());
	}
}

