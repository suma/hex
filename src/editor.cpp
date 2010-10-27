
#include <QtGui>
#include "editor.h"
#include "control/standard.h"
#include "control/document.h"
#include "control/highlight.h"

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

	high_ = NULL;
	//high_ = new Highlight(doc_);

	//cur_ = new Standard::Cursor(doc_);
	hview_ = new Standard::HexView(this, doc_, high_);
	hview_->setCaretBlink(true);

	
	/*
	aview_ = new Standard::AddressView(this, doc_, NULL);
	aview_->move(0,0);
	aview_->resize(0, 270);
	*/

	resize(900,400);
	hview_->move(0,0);
	hview_->resize(hview_->getConfig().width(), 270);
	qDebug("%d ", hview_->getConfig().width());
	hview_->show();

	tview_ = new Standard::TextView(this, doc_, high_);
	tview_->setCaretBlink(true);
	tview_->move(hview_->getConfig().width() + 20, 0);
	tview_->resize(tview_->getConfig().width(), 270);
	tview_->show();

	////connect(hview_, SIGNAL(viewDrawed(DrawMode, int, int)), tview_, SLOT(drawView(DrawMode, int ,int)));
	//connect(hview_, SIGNAL(viewDrawed(DrawMode, int, int)),
	//	tview_, SLOT(drawView(DrawMode, int, int)));



	//aview_->show();



	// ugoku!!
	//isetWindowOpacity(0.8);
	//setMask(QRect(0,10,width(), height()));

	//setBackgroundRole(QPalette::Base);
	//QWidget::setAttribute(Qt::WA_PaintOnScreen);
	//setBackgroundRole(QPalette::NoRole);

	//setMask(QPixmap(size()).mask());
	//aview_->setPalette(QPalette(Qt::transparent));
	/*
	setWindowOpacity(0.3);
	setAttribute( Qt::WA_OpaquePaintEvent );
	setBackgroundRole(QPalette::NoRole);
	setPalette(QPalette(Qt::transparent));

	QPalette p = palette();
	p.setBrush(QPalette::Base, QColor(0, 0, 0, 
	  		127));
	setPalette(p);
	*/

}

void Editor::resizeEvent(QResizeEvent *resize)
{
	if (hview_) {
		hview_->resize(hview_->width(), height());
	}
	if (tview_) {
		tview_->resize(tview_->getConfig().width(), height());
	}
}

