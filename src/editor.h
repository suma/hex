#ifndef EDITOR_H_INC
#define EDITOR_H_INC

#include <QWidget>
#include <QLabel>

class Document;

namespace Standard {
	class HexView;
	class TextView;
	class AddressView;
	class Cursor;
}

class Highlight;

class Editor : public QWidget
{
	Q_OBJECT

public:
	Editor();

protected:

	void resizeEvent(QResizeEvent*);

protected:
	QLabel *label;
	Document *doc_;
	Highlight *high_;
	Standard::HexView *hview_;
	Standard::TextView *tview_;
	Standard::AddressView *aview_;
	//Standard::Cursor *cur_;

};

#endif
