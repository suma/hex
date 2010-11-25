
#include <QtGui>
#include "addressview.h"
#include "hexview.h"
#include "textview.h"
#include "cursor.h"
#include "../util/util.h"

namespace Standard {


AddressConfig::AddressConfig()
	: num_(16)
	, margin_(2, 2, 3, 3)
	, byteMargin_(3, 0, 2, 0)
	, font_("Monaco", 17)
	, fontMetrics_(font_)
{
	// Coloring
	colors_[Color::Background] = QColor(0xCC,0xFF,0xFF);
	colors_[Color::Text] = QColor(0,0,0);
	colors_[Color::SelBackground] = QColor(0xFF,0xA0,0xFF);
	colors_[Color::SelText] = QColor(0,0,0);

}

AddressConfig::~AddressConfig()
{
}

uint AddressConfig::num() const
{
	return num_;
}

QFont AddressConfig::font() const
{
	return font_;
}

const QFontMetrics &AddressConfig::fontMetrics() const
{
	return fontMetrics_;
}

QColor AddressConfig::color(size_t index) const
{
	Q_ASSERT(index < Color::ColorCount);
	return colors_[index];
}

QRect AddressConfig::margin() const
{
	return margin_;
}

QRect AddressConfig::byteMargin() const
{
	return byteMargin_;
}

int AddressConfig::top() const
{
	return margin_.top();
}

int AddressConfig::byteHeight() const
{
	return byteMargin_.top() + fontMetrics_.height() + byteMargin_.bottom();
}

void AddressConfig::setFont(QFont font)
{
	if (font_ != font) {
		font_ = font;
		emit fontChanged(font);
	}
}

void AddressConfig::setNum(int num)
{
	if (num_ != num) {
		num_ = num;
		emit numChanged(num);
	}
}

int AddressConfig::drawableLines(int height) const
{
	const int y = top() + byteMargin_.top();
	return (height - y + byteHeight()) / byteHeight();
}


AddressView::AddressView(QWidget *parent, ::Document *doc, HexView *view)
	: QWidget(parent)
	, document_(doc)
	, view_(view)
	, cursor_(view->cursor())
{
	// connect slot
	QObject::connect(&cursor_, SIGNAL(topChanged(quint64)), this, SLOT(topChanged(quint64)));
	QObject::connect(&cursor_, SIGNAL(positionChanged(quint64, quint64)), this, SLOT(positionChanged(quint64, quint64)));


}

AddressView::~AddressView()
{
}

void AddressView::paintEvent(QPaintEvent *ev)
{
	// needs to be updated
	//ev->rect();
	drawView();

}

void AddressView::drawView()
{
	QPainter painter(this);
	painter.setFont(config_.font());

	const quint64 top = cursor_.top() * config_.num();
	quint64 line = top;

	int count = config_.drawableLines(height());
	int y = config_.top() + config_.byteMargin().top();

	const quint64 sel_pos = cursor_.position() / config_.num();

	QString str;
	str.resize(8);
	for (int i = 0; i < count; i++) {
		for (quint32 j = 0, addr = (line & 0xFFFFFFFF); j < 8; j++) {
			str[7 - j] = QChar(util::itohex(addr & 0xF));
			addr >>= 4;
		}
		qDebug() << line << sel_pos;
		// Change cursor color
		if ((line / config_.num()) == sel_pos) {
			painter.setPen(config_.color(Color::SelText));
			painter.setBackground(QBrush(config_.color(Color::SelBackground)));
			painter.fillRect(0, y, config_.fontMetrics().width(str), config_.byteHeight(), QBrush(config_.color(Color::SelBackground)));
		} else {
			painter.setPen(config_.color(Color::Text));
			painter.setBackground(QBrush(config_.color(Color::Background)));
		}
		painter.drawText(0, y, config_.fontMetrics().width(str), config_.byteHeight(), Qt::AlignLeft, str);

		y += config_.byteHeight();
		line += config_.num();
	}
}

void AddressView::topChanged(quint64)
{
	repaint();
}

void AddressView::positionChanged(quint64, quint64)
{
	repaint();
}


}	// namespace

