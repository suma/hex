
#pragma once

#include <QWidget>
#include <QLabel>

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
	Editor();

protected:

	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent *ev);

protected:
	QLabel *label;
	Document *doc_;
	Standard::HexView *hview_;
	Standard::TextView *tview_;
	Standard::AddressView *aview_;

};


