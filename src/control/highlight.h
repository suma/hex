#ifndef HIGHLIGHT_H_INC
#define HIGHLIGHT_H_INC

#include <QColor>
#include <vector>
#include "color.h"

class Document;

struct ColorInfo {
	uint Length;
	QColor *Colors;
	ColorInfo(uint length, QColor *col) {
		Length = length;
		Colors = col;
	}
};

typedef std::vector<ColorInfo> CIList;

class Highlight
{
protected:
	Document *doc_;

public:
	Highlight(Document *doc);
	~Highlight();

	virtual void refresh();
	virtual bool GetColor(std::vector<uchar> &buf, quint64 pos, uint size, CIList &list);	// return Do or Do not highlighting

};



#endif
