
#include "highlight.h"
#include "document.h"

using namespace std;

Highlight::Highlight(Document *doc)
	: doc_(doc)
{
}

Highlight::~Highlight()
{
}

void Highlight::refresh()
{
}

bool Highlight::GetColor(vector<uchar> &buf, quint64 pos, uint size, HCIList &list)
{
	QColor c[] = {
		QColor(0xFF, 0xCC, 0xCC), QColor(0x00, 0x00, 0xFF),
		QColor(0, 0xCC, 0xFF), QColor(0xCC, 0, 0xFF)
	};
	QColor d[] = {
		QColor(0xFF, 0xFF, 0xFF), QColor(0, 0, 0),
		QColor(0x00, 0xFF, 0x00), QColor(0, 0, 0)
	};
	QColor e[] = {
		QColor(0xCC, 0xFF, 0xFF), QColor(0, 0, 0),
		QColor(0x00, 0x00, 0xFF), QColor(0, 0, 0)
	};

	if (0) {
		list.clear();
		for (int i = 0; i < size; i++) {
			if (buf[i] == 0xFF) {
				qDebug("0xFF: %d", i);
				list.push_back(HighlightColorInfo(i, 1, c));
			}
		}
		return true;
	}
	if (1) {
		int i = 0, l = ((pos + i) / 16) & 1;
		while (i + 16 <= size) {
			list.push_back(HighlightColorInfo(i, 16, l & 1 ? d : e));
			qDebug("GetColor i:%d l:%d", i, l);
			i += 16;
			l++;
		}
		if (size - i) {
			list.push_back(HighlightColorInfo(i, size - i, l & 1 ? d: e));
			qDebug("GetColor i:%d l:%d l: %d", i, l, size - i);
		}
		return true;
	}
	// sum(list[0].length, ..., list[N].length) = size とならなくてよい
	return false;
}

