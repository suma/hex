#ifndef HIGHLIGHT_H_INC
#define HIGHLIGHT_H_INC

#include <QColor>
#include <vector>
#include "color.h"

class Document;

struct HighlightColorInfo {
	uint Index, Length;
	QColor Colors[Color::ColorCount];
	HighlightColorInfo(uint index, uint length, QColor *col)
	{
		Index = index;
		Length = length;
		memcpy(Colors, col, sizeof(Colors));
	}
};

typedef std::vector<HighlightColorInfo> HCIList;

class Highlight
{
protected:
	Document *doc_;

public:
	Highlight(Document *doc);
	~Highlight();

	virtual void refresh();
	virtual bool GetColor(std::vector<uchar> &buf, quint64 pos, uint size, HCIList &list);	// return Do or Do not highlighting

};



#endif
