
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

void View::resizeEvent(QResizeEvent *rs)
{
	if (pix_.size().width() < rs->size().width() ||
		pix_.size().height() < rs->size().height()) {
		pix_ = QPixmap(rs->size());
	}
}

void View::getDrawColors(const DrawInfo &di, DCIList &ci)
{
	quint64 top = di.top;
	const int sb = di.sb, se = di.se;
	const uint size = di.size;

	bool high = false;
	if (high_ != NULL && high_->GetColor(buff_, top, size, hcolors_)) {
		qDebug("hcolors_.size(): %d", hcolors_.size());
		high = true;
	}

	// clear
	ci.clear();

	qDebug("selected: %d, highligh on: %d", di.selected, high);
	if (!di.selected) {
		// case: Not selected
		if (high) {
			// case: Highlight off
			// Scanning highlighted colors
			int i = 0;
			for (HCIList::iterator itr = hcolors_.begin(), end = hcolors_.end(); i < size; ) {
				if (itr == end || i < itr->Index) {
					int left = itr->Index - size;
					ci.push_back(DrawColorInfo(left));
					i += left;
				} else {
					ci.push_back(DrawColorInfo(itr->Length));
					i += itr->Length;
					++itr;
				}
			}
			// Left
			if (i < size) {
				ci.push_back(DrawColorInfo(size - i));
			}
		} else {
			// case: Highlight off
			ci.push_back(DrawColorInfo(size));
		}
	} else {

		// buggy(highlight)
		// case: Selected
		quint64 index = top;
		// check colors
		qDebug("sb: %d, se: %d", sb, se);
		int i = 0;
		int last = -1;
		for (; i < size; i++, index++) {
			int index_color = (sb <= index && index < se) ? Color::SelBackground : 0;
			if (last != index_color) {
				last = index_color;
				ci.push_back(DrawColorInfo(1, Color::Background + index_color, Color::Text + index_color));
				continue;
			}
			// continues same color
			ci.back().Length++;
		}
		/*
		bool l_out = false;
		for (HCIList::iterator itr = hcolors_.begin(), end = hcolors_.end(), last = end; i < size; i++, index++) {
			bool sel = sb <= index && index < se;
			int x = sel ? 2 : 0;
			if (itr != end && itr->Index <= i && i < itr->Index + itr->Length) {
				// inner itr
				if (itr == last) {
					// continues same color
					ci.back().Length++;
				} else {
					qDebug("i:%d index:%d len:%d\n", i, itr->Index, itr->Length);
					ci.push_back(DrawColorInfo(1, itr->Colors + x));
				}
				itr->Length--;
				if (itr->Length == 0) {
					++itr;
				}
				l_out = false;
			} else {
				// out of itr
				if (l_out) {
					// continues same color
					ci.back().Length++;
				} else {
					ci.push_back(DrawColorInfo(1, defColors + 2));
					l_out = true;
				}
			}
		}
		*/
	}
}



