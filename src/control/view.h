#ifndef VIEW_H_INC
#define VIEW_H_INC

#include <QWidget>

class Document;

namespace Color {
	enum color {
		Background = 0,
		Text,
		HiBackground,	// hilighted
		HiSelText,
		ColorCount,
	};
}


class View : public QWidget
{
	Q_OBJECT

protected:
	Document *doc_;
	QPixmap pix_;

public:
	View(QWidget *parent = 0, Document *doc = 0);

protected:
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent*);

protected:
	virtual void refreshPixmap() = 0;

};



#endif
