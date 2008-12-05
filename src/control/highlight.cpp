
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
	if (1) {
		list.clear();
		for (int i = 0; i < size; i++) {
			QColor c[] = { QColor(0xFF, 0xCC, 0xCC), QColor(0x00, 0x00, 0xFF), QColor(0, 0xCC, 0xFF), QColor(0xCC, 0, 0xFF) };
			if (buf[i] == 0xFF) {
				qDebug("0xFF: %d\n", i);
				list.push_back(HighlightColorInfo(i, 1, c));
			}
		}
		return true;
	}
	// sum(list[0].length, ..., list[N].length) = size とならなくてよい
	return false;
}

