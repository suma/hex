#ifndef VIEW_H_INC
#define VIEW_H_INC

#include <QWidget>
#include <vector>
#include "color.h"
#include "highlight.h"

class Document;

struct DrawInfo {
	int y;
	int count;
	int xb;
	int xe;
	uint size;
	quint64 top;
	quint64 sb;
	quint64 se;
	bool selected;
	DrawInfo(int Y, quint64 Top, int Count, int Xb, int Xe, quint64 Sb, quint64 Se, uint Size, bool sel)
	{
		y = Y;
		count = Count;
		xb = Xb;
		Xe = Xe;
		size = Size;
		top = Top;
		sb = Sb;
		se = Se;
		selected = sel;
	}
	DrawInfo() {}
	~DrawInfo() {}
};

class View : public QWidget
{
	Q_OBJECT

protected:
	Document *doc_;
	QPixmap pix_;
	Highlight *high_;

	// Temporary buffer
	std::vector<uchar> buff_;
	std::vector<ColorInfo> colors_;

public:
	View(QWidget *parent = NULL, Document *doc = NULL, Highlight *hi = NULL);

protected:
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent*);
	void getDrawColors(const DrawInfo &di, std::vector<ColorInfo> &ci, QColor *defaultColors);

protected:
	virtual void refreshPixmap() = 0;

};



#endif
