
#include <QtGui>
#include "view.h"
#include "document.h"

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


void View::getDrawColors(const DrawInfo &di, std::vector<ColorInfo> &ci)
{
	int y = di.y;
	quint64 top = di.top;
	const int sb = di.sb, se = di.se;
	const uint size = di.size;

	if (!di.selected) {
		// non selected
		if (high_ == NULL || !high_->GetColor(buff_, top, size, colors_)) {
			//DrawInfo di(ALL, y, lines, width_count);
		} else {
		}
		return;
	} else {
		// selected
		quint64 index = top;
		if (high_ == NULL || !high_->GetColor(buff_, top, size, colors_)) {
			// case:
			// 4.B only
			// 2.B and W
			// 3.W and B
			// 1.W and B and W
			qint64 diff = sb - index;
			qint64 left = se - index;
		} else {
			// check colors
			for (int i = 0; i < size; i++, index++) {
				qint64 diff = sb - index;
				qint64 left = se - index;
				if (0 < diff && diff < 16) {
					// begin after
				} else if (0 < left && left < 16) {
					// end after
				} else if (diff <= 0 && left <= 0) {
					// equal (sb <= index && index <= se)
					// inner selected
				} else {
					// normal
				}
			}
		}
	}
}



