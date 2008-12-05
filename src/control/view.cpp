
#include <QtGui>
#include <algorithm>
#include "view.h"
#include "document.h"

using namespace std;

View::View(QWidget *parent, Document *doc, Highlight *hi)
	: QWidget(parent)
	, doc_(doc)
	, high_(hi)
{
}

void View::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, pix_);
}

void View::resizeEvent(QResizeEvent *)
{
	pix_ = QPixmap(size());
	refreshPixmap();
}

void View::getDrawColors(const DrawInfo &di, DCIList &ci, QColor *defColors)
{
	Q_ASSERT(defColors != NULL);

	quint64 top = di.top;
	const int sb = di.sb, se = di.se;
	const uint size = di.size;

	bool high = false;
	if (high_ != NULL && high_->GetColor(buff_, top, size, hcolors_)) {
		qDebug("hcolors_.size(): %d\n", hcolors_.size());
		high = true;
	}

	// clear
	ci.clear();

	qDebug("selected: %d, highligh on: %d\n", di.selected, high);
	if (!di.selected) {
		// case: Not selected
		if (high) {
			// case: Highlight off
			// Scanning highlighted colors
			int i = 0;
			for (HCIList::iterator itr = hcolors_.begin(), end = hcolors_.end(); i < size; ) {
				if (itr == end || i < itr->Index) {
					int left = itr->Index - size;
					ci.push_back(DrawColorInfo(left, defColors));
					i += left;
				} else {
					ci.push_back(DrawColorInfo(itr->Length, itr->Colors));
					i += itr->Length;
					++itr;
				}
			}
			// Left
			if (i < size) {
				ci.push_back(DrawColorInfo(size - i, defColors));
			}
		} else {
			// case: Highlight off
			ci.push_back(DrawColorInfo(size, defColors));
		}
	} else {
		// case: Selected
		quint64 index = top;
		// check colors
		qint64 diff = sb - index;
		qint64 left = se - index;
		qDebug("sb: %d, se: %d\n", sb, se);
		int i = 0;
		QColor *last = NULL;
		for (HCIList::iterator itr = hcolors_.begin(), end = hcolors_.end(); i < size; i++, index++, diff--, left--) {
			bool sel = sb <= index && index <= se;
			int x = sel ? 2 : 0;
			if (itr == end || i < itr->Index || itr->Index + itr->Length < i) {
				// out of itr
				if (last == defColors + x) {
					// continues same color
					ci.back().Length++;
				} else {
					last = defColors + x;
					ci.push_back(DrawColorInfo(1, last));
				}
			} else {
				// inner itr
				if (last == itr->Colors + x) {
					// continues same color
					ci.back().Length++;
				} else {
					last = itr->Colors + x;
					ci.push_back(DrawColorInfo(1, last));
				}
				itr->Length--;
				if (itr->Length == 0) {
					++itr;
				}
			}
		}
	}
}



