#ifndef VIEW_H_INC
#define VIEW_H_INC

#include <QWidget>
#include <vector>
#include "color.h"
#include "highlight.h"

class Document;

struct DrawInfo {
	int y;
	uint size;
	quint64 top;
	quint64 sb;
	quint64 se;
	bool selected;
	DrawInfo(int Y, quint64 Top, quint64 Sb, quint64 Se, uint Size, bool sel)
	{
		y = Y;
		size = Size;
		top = Top;
		sb = Sb;
		se = Se;
		selected = sel;
	}
	DrawInfo() {}
	~DrawInfo() {}
};

struct DrawColorInfo {
	uint Length;
	int BackgroundColor;
	int TextColor;
	DrawColorInfo(uint length, int bg_color = Color::Background, int text_color = Color::Text)
	{
		Length = length;
		BackgroundColor = bg_color;
		TextColor = text_color;
	}
};

typedef std::vector<DrawColorInfo> DCIList;

// TODO: add caret support
//       caret width, height, x, y
//       caret color(QColor with Alpha)
//       redraw support

class View : public QWidget
{
	Q_OBJECT

protected:
	Document *document;
	QPixmap pix_;
	Highlight *high_;

	// Temporary buffer
	std::vector<uchar> buff_;
	HCIList hcolors_;
	DCIList dcolors_;

public:
	View(QWidget *parent = NULL, Document *doc = NULL, Highlight *hi = NULL);

protected:
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent*);
	void getDrawColor(const DrawInfo &di, DrawColorInfo &dci, uchar data);
	void getDrawColors(const DrawInfo &di, DCIList &ci);

protected:
	virtual void refreshPixmap() = 0;

};



#endif
