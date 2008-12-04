#ifndef HIGHLIGHT_H_INC
#define HIGHLIGHT_H_INC

#include <QColor>
#include <vector>
#include "color.h"

class Document;

struct ColorInfo {
	uint Length;
	QColor colors[Color::ColorCount];
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
