
#pragma once

#include <QWidget>
#include "control/standard.h"

class Document;

namespace Standard {
	class HexView;
	class TextView;
	class AddressView;
}


class Editor : public QWidget
{
	Q_OBJECT

public:
	Editor(QWidget *parent = NULL);
	Editor(QWidget *parent, Document *document);
	Document *document() const;

protected:

	void initView();

	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent *ev);

protected:
	Document *document_;
	Standard::AddressView *view_;

};


