
#include <QtGui>
#include "global.h"
#include "addressview.h"
#include "layeredwidget.h"
#include "hexview.h"
#include "textview.h"
#include "cursor.h"
#include "../util/util.h"
#include "../scrollbar.h"

namespace Standard {


AddressConfig::AddressConfig(Global *global)
	: global_(global)
	, byteMargin_(3, 0, 2, 0)
	, column_visible_(true)
	, line_visible_(true)
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

QFont AddressConfig::font() const
{
	return global_->config().font();
}

const QFontMetrics &AddressConfig::fontMetrics() const
{
	return global_->config().fontMetrics();
}

QColor AddressConfig::color(size_t index) const
{
	Q_ASSERT(index < Color::ColorCount);
	return colors_[index];
}

uint AddressConfig::num() const
{
	return global_->config().num();
}

QRect AddressConfig::margin() const
{
	return global_->config().margin();
}

QRect AddressConfig::byteMargin() const
{
	return byteMargin_;
}

int AddressConfig::top() const
{
	return margin().top();
}
int AddressConfig::charWidth(int num) const
{
	return global_->config().charWidth(num);
}

int AddressConfig::byteHeight() const
{
	return global_->config().byteHeight();
}

int AddressConfig::columnHeight() const
{
	return fontMetrics().height();
}

bool AddressConfig::columnVisible() const
{
	return column_visible_;
}

bool AddressConfig::lineVisible() const
{
	return line_visible_;
}

void AddressConfig::setColumnVisible(bool visible)
{
	column_visible_ = visible;
}

void AddressConfig::setLineVisible(bool visible)
{
	line_visible_ = visible;
}

int AddressConfig::drawableLines(int height) const
{
	const int y = top() + byteMargin_.top() + (columnVisible() ? columnHeight() : 0);
	return (height - y + byteHeight()) / byteHeight();
}


AddressView::AddressView(QWidget *parent, Global *global)
	: QWidget(parent)
	, config_(global)
	, global_(global)
	, scrollbar_(new ::ScrollBar(Qt::Vertical, parent))
	, document_(global->document())
	, cursor_(new Cursor)
	, last_focus_(NULL)
	, hex_(NULL)
	, text_(NULL)
	, hex_layer_(new LayeredWidget(parent))
	, text_layer_(new LayeredWidget(parent))
	, column_visible_(true)
	, line_visible_(true)
{
	// connect cursor slot
	QObject::connect(cursor_, SIGNAL(topChanged(quint64)), this, SLOT(topChanged(quint64)));
	QObject::connect(cursor_, SIGNAL(positionChanged(quint64, quint64)), this, SLOT(positionChanged(quint64, quint64)));

	// connect scrollbar slot
	QObject::connect(scrollbar_, SIGNAL(valueChanged(qint64)), this, SLOT(valueChanged(qint64)));

	// connect document event slot
	QObject::connect(document_, SIGNAL(dataChanged()), this, SLOT(documentChanged()));

	last_focus_ = hex_layer_;
}

AddressView::~AddressView()
{
}

void AddressView::connect(Cursor *cursor)
{
	// connect each other
	cursor->connectTo(cursor_);
	cursor_->connectTo(cursor);
}

void AddressView::setHexView(HexView *hex)
{
	if (hex != hex_) {
		if (hex_ != NULL) {
			hex_->removeEventFilter(this);
		}
		hex_ = hex;
		hex_->installEventFilter(this);
		hex_layer_->add(hex);
		update();
	}
}

void AddressView::setTextView(TextView *text)
{
	if (text != text_) {
		if (text_ != NULL) {
			text_->removeEventFilter(this);
		}
		text_ = text;
		text_->installEventFilter(this);
		text_layer_->add(text);
		update();
	}
}

void AddressView::addHex(QWidget *widget)
{
	if (hex_) {
		hex_->add(widget);
	}
}

void AddressView::addHexUnder(QWidget *widget)
{
	hex_layer_->add(widget);
	if (hex_ != NULL) {
		widget->stackUnder(hex_);
	}
}

void AddressView::addText(QWidget *widget)
{
	if (text_) {
		text_->add(widget);
	}
}

void AddressView::addTextUnder(QWidget *widget)
{
	text_layer_->add(widget);
	if (text_ != NULL) {
		widget->stackUnder(text_);
	}
}

HexView *AddressView::hexView() const
{
	return hex_;
}

TextView *AddressView::textView() const
{
	return text_;
}

void AddressView::paintEvent(QPaintEvent *)
{
	// FIXME: check update region

	// needs to be updated
	if (config_.columnVisible()) {
		drawColumn();
	}

	if (config_.lineVisible()) {
		drawLine();
	}
}

void AddressView::focusInEvent(QFocusEvent *)
{
	if (last_focus_ != NULL) {
		last_focus_->setFocus(Qt::OtherFocusReason);
	}
}

bool AddressView::eventFilter(QObject *obj, QEvent *event)
{
	QWidget *widget = NULL;
	if (!obj->isWidgetType()) {
		goto standard_processing;
	}

	// Classfy which view(hex/text) focused in
	widget = dynamic_cast<QWidget*>(obj);
	if (event->type() == QEvent::FocusIn) {
		last_focus_ = widget;
	}

standard_processing:
	return QObject::eventFilter(obj, event);
}

void AddressView::drawColumn()
{
	QPainter painter(this);
	painter.setFont(config_.font());

	if (hex_ == NULL && text_ == NULL) {
		return;
	}

	const int Num = static_cast<int>(config_.num());
	const int hx = hexPos();
	const int sel_pos = static_cast<int>(cursor_->position() % config_.num());
	
	// draw hex column
	if (hex_ != NULL) {
		HexConfig &hc = hex_->config();
		HexConfig::XIterator hi = hc.createXIterator();
		for (int i = 0; i < Num; i++, ++hi) {
			if (i == sel_pos) {
				painter.setPen(config_.color(Color::SelText));
				painter.setBackground(QBrush(config_.color(Color::SelBackground)));
			} else {
				painter.setPen(config_.color(Color::Text));
				painter.setBackground(QBrush(config_.color(Color::Background)));
			}
			QString str("+");
			str += QChar(util::itohex(i));

			// draw column background
			if (i == sel_pos) {
				painter.fillRect(hx + hi.screenX(), 0, hc.byteWidth(), hc.byteHeight(), QBrush(config_.color(Color::SelBackground)));
			}
			// draw text
			painter.drawText(hx + hi.textX(), 0, hc.fontMetrics().width(str), config_.byteHeight(), Qt::AlignLeft, str);
		}
	}
	
	// TODO: draw text column
	//const int tx = textPos();
}

void AddressView::drawLine()
{
	QPainter painter(this);
	painter.setFont(config_.font());

	const quint64 top = cursor_->top() * config_.num();
	quint64 line = top;

	int count = config_.drawableLines(height());
	int y = config_.top() + config_.byteMargin().top();
	if (config_.lineVisible()) {
		y += config_.columnHeight();
	}

	const quint64 sel_pos = cursor_->position() / config_.num();

	QString str;
	str.resize(8);
	for (int i = 0; i < count; i++) {
		for (quint32 j = 0, addr = (line & 0xFFFFFFFF); j < 8; j++) {
			str[7 - j] = QChar(util::itohex(addr & 0xF));
			addr >>= 4;
		}
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

void AddressView::topChanged(quint64 top)
{
	// Sync to value of ScrollBar
	scrollbar_->setValue(static_cast<qint64>(top));
	update();
}

void AddressView::positionChanged(quint64, quint64)
{
	update();
}

void AddressView::valueChanged(qint64 value)
{
	// Scrollbar value changed
	cursor_->setTop(static_cast<quint64>(value));
}

void AddressView::childEvent(QChildEvent *)
{
}

void AddressView::resizeEvent(QResizeEvent *)
{
	const int h = height() - (config_.columnVisible() ? config_.columnHeight() : 0);
	if (hex_ != NULL) {
		hex_layer_->move(hexPos(), y());
		hex_layer_->resize(hex_->config().width(), h);
	}
	if (text_ != NULL) {
		text_layer_->move(textPos(), y());
		text_layer_->resize(text_->config().width(), h);
	}

	// ScrollBar
	const int scroll_width = scrollbar_->sizeHint().width();
	scrollbar_->move(width() - scroll_width, 0);
	scrollbar_->resize(scroll_width, height());

	// Refresh ScrollBar info
	refreshScrollbarInfo();
}

int AddressView::hexPos() const
{
	return config_.lineVisible() ? config_.charWidth(8) : 0;
}

int AddressView::textPos() const
{
	return hexPos() + (hex_ == NULL ? 0 : hex_->config().width());
}

int AddressView::y() const
{
	return config_.columnVisible() ? config_.columnHeight() : 0;
}

void AddressView::refreshScrollbarInfo()
{
	int pageStep = config_.drawableLines(height());
	if (pageStep > 0) {
		pageStep = qMax(pageStep - 1, 0);
		const qint64 modulo = document_->length() % config_.num();
		qint64 maximum = document_->length() / config_.num();
		if (modulo || maximum == pageStep) {
			maximum++;
		}
		maximum = maximum - pageStep;
		scrollbar_->setMinimum(0);
		scrollbar_->setMaximum(maximum);
		scrollbar_->setPageStep(pageStep);
	}
}

void AddressView::documentChanged()
{
	// TODO: chaching document length
	refreshScrollbarInfo();
}


}	// namespace Standard

