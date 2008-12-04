
#include <QtGui>
#include <algorithm>
#include <vector>
#include "hexview.h"
#include "scursor.h"
#include "../document.h"
#include "../highlight.h"

using namespace std;

namespace Standard {

////////////////////////////////////////
// Config
HexConfig::HexConfig()
	: Margin(10, 10, 10, 10)
	, ByteMargin(0, 4, 5, 1)
	, Font("times", 24)
	, FontMetrics(Font)
{
	Colors[Color::Background] = QColor(255,0,0);
	Colors[Color::Text] = QColor(0,0,255);

	for (int i = 1; i < Num; i++) {
		Spaces[i] = FontMetrics.maxWidth();
	}
	Spaces[0] = Spaces[Num] = 0;
	Spaces[Num / 2] *= 2;

	calculate();
}

void HexConfig::calculate()
{
	// Pos
	x_[0] = Margin.left() + ByteMargin.left();
	for (int i = 1; i < Num; i++) {
		x_[i] = x_[i-1] + byteWidth() + Spaces[i];
	}

	// Area
	xarea_[0] = Margin.left();
	for (int i = 1; i < Num; i++) {
		xarea_[i] = xarea_[i-1] + byteWidth() + Spaces[i];
	}
	for (int i = 1; i < Num; i++) {
		xarea_[i] -= Spaces[i] / 2;
	}

	top_ = Margin.top() + ByteMargin.top();
}

int HexConfig::toPos(int x)
{
	if (x < Margin.left()) {
		return -1;
	}

	int a = (int)distance(xarea_, lower_bound(xarea_, xarea_ + Num + 1, x));
	if (a == Num + 1) {
		return 32;
	}

	if (x <= xarea_[a] + FontMetrics.maxWidth()) {
		return x * 2 + 1;
	}
	return x * 2;
}

int HexConfig::toLine(int y)
{
}

////////////////////////////////////////
// View
HexView::HexView(QWidget *parent, Document *doc, Cursor *cur, Highlight *hi)
	: ::View(parent, doc)
	, cur_(cur)
	, high_(hi)
{
	setWindowOpacity(0.5);
}

void HexView::refreshPixmap()
{
	refreshPixmap(0);
}

void HexView::refreshPixmap(int)
{
	//
	// TODO: Optimizing drawing

	pix_.fill(config_.Colors[Color::Background]);
	QPainter painter(&pix_);
	//painter.initFrom(this);

	painter.drawText(20, 20, QString("abcdefg"));

	/* sel.end < vpostop => not selected
	 * vposend < sel.begin => not selected
	 *
	 * 4 patterns
	 * begin >= top  :: 最初から
	 * begin != top  :: 途中から
	 * end <= bottom :: 最後まで
	 * end != bottom :: 途中まで
	 *
	 */

	if (!doc_->length()) {
		// draw Empty Background only
		return;
	}

	int y = config_.top();
	const int yMax = height();
	const int yCount = (height() - y + config_.byteHeight()) / config_.byteHeight();
	quint64 top = cur_->Top * 16;
	const int xb = 0, xe = width();
	const uint size = min(doc_->length() - top, 16ULL * yCount);

	bool selected = false;
	quint64 sb = 0, se = 0;
	if (cur_->Selected) {
		sb = max(cur_->SelBegin, cur_->SelEnd);
		se = min(cur_->SelBegin, cur_->SelEnd);
		if (top <= se) {
			const quint64 vpos_end = max(top + (16ULL * yCount), top + size);
			if (sb <= vpos_end) {
				selected = true;
			}
		}
	}

	if (buff_.capacity() < size) {
		buff_.reserve(size);
	}
	doc_->get(top, &buff_[0], size);

	// Draw
	DrawInfo di(y, top, yCount, xb, xe, sb, se, size);
	if (selected) {
	} else {
	}

	// ALL
	// LINE
	// AFTER, line

	// draw lines
	/*
	int y = config_.top();
	const int yMax = height();
	const int yCount = (height() - y + config_.byteHeight()) / config_.byteHeight();
	Q_ASSERT(doc_->length() >= (cur_->Top * 16));
	const quint64 top = cur_->Top * 16;
	const uint size = min(doc_->length() - top, 16ULL * yCount);
	vector<uchar> buff(size);
	*/


	update();
}

// x: [xb, xe)
// y: [line, line_e)
// line: line begin
// line_e: line end
// xb: x begin
// xe: x end
/*
int y;
int count;
int xb;
int xe;
uint size;
quint64 top;
quint64 sb;
quint64 se;
std::vector<uchar> buff;
*/
void HexView::drawSelected(const DrawInfo &di)
{
	// 1. draw bakcground: selected/nonselected/hilighted

	// 2. draw hex 

	// -: nonselected
	// o: selected
	// case:
	// a [-------]
	// b [ooooooo]
	// c [oooo---]
	// d [---oooo]
	// e [--ooo--]
	/*
	bool sel = false;
	st = case()
	swich (st) {
	case a:
		push(A)
		if (sel) {
			RENZOKU
		} else {
			A/D/E
		}
	case b: sel = true
		// CHECK RENZOKU
		// else A/C/D/E
	case c: sel = true
		// CHECK A
	case d: sel = true
		// CHECK B/C
	case e: sel = true
		// CHECK A
	}
*/

	// o: selected
	// -: nonselected
	// case Y
	//  [ooooooo]
	//  [oooo---]
	//  [---oooo]
	//  [--ooo--]
	// case X
	//  [ooooooo] on
	//  [oooo---] on + non
	//  [---oooo] non + on
	//  [--ooo--] non + on + non
	//  [-------] nonselected

}

#ifdef HOGEHOGE_DEBUG
struct ColorInfo {
	quint64 Length;
	Color[ColorCount]
};

struct DrawInfo {
	// type: all, piece
	int line;
	int size;
	QColor color[2];	// text and background
}

struct DrawHighlightInfo {
	// type: all, piece
	int line;
	int size;
	bool continuous;	// front of line
	QColor color;
};

{
	int y = config_.top();
	const int yMax = height();
	const int yCount = (height() - y + config_.byteHeight()) / config_.byteHeight();
	quint64 top = cur_->Top * 16;
	const int xb = 0, xe = width();
	const uint size = min(doc_->length() - top, 16ULL * yCount);

	bool selected = false;
	quint64 sb = 0, se = 0;
	if (cur_->Selected) {
		sb = max(cur_->SelBegin, cur_->SelEnd);
		se = min(cur_->SelBegin, cur_->SelEnd);
		if (top <= se) {
			const quint64 vpos_end = max(top + (16ULL * yCount), top + size);
			if (sb <= vpos_end) {
				selected = true;
			}
		}
	}

	if (!selected) {
		// non selected
		if (!high_->GetPosColor(buff_, top, size, colors_)) {
			DrawInfo di(ALL, y, lines, width_count);
		} else {
		}
		return;
	} else {
		// selected
		quint64 index = top;
		if (high_ == NULL || !high_->GetPosColor(buff_, top, size, colors_)) {
			// case:
			// B only
			// B and W
			// W and B
			// W and B and W
			qint64 diff = sb - index;
			qint64 left = se - index;
		} else {
			// check colors
			for (int i = 0, j = 0; i < lines; i++, j = (j+1) & 0xF, index++) {
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
#endif

void HexView::drawNonSelected(const DrawInfo &di)
{
	/*
	Q_ASSERT(doc_->length() >= (top * 16));
	const uint size = min(doc_->length() - top, 16ULL * count);
	vector<uchar> buff(size);
	doc_->get(top, &buff[0], size);
	*/

	//for (int i = 0; i < count; i++) {
	//}
}

void HexView::mousePressEvent(QMouseEvent*)
{
}

void HexView::mouseMoveEvent(QMouseEvent*)
{
}

void HexView::mouseReleaseEvent(QMouseEvent*)
{
}


}	// namespace
