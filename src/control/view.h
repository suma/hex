#ifndef VIEW_H_INC
#define VIEW_H_INC

#include <QWidget>
#include <vector>
#include "color.h"
#include "highlight.h"

class Document;

struct ColorType {
	int Background;
	int Text;
	ColorType(int bg_color = Color::Background, int text_color = Color::Text)
	{
		Background = bg_color;
		Text = text_color;
	}
};

class View : public QWidget
{
	Q_OBJECT

protected:
	Document *document_;
	QPixmap pix_;
	Highlight *high_;

	// Temporary buffer
	std::vector<uchar> buff_;

public:
	View(QWidget *parent = NULL, Document *doc = NULL, Highlight *hi = NULL);
	virtual ~View();

private:
	void paintEvent(QPaintEvent*);

protected:
	void resizeEvent(QResizeEvent*);
};



#endif
