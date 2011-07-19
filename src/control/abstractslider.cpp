
#include "abstractslider.h"

AbstractSlider::AbstractSlider(QWidget *parent)
	: QWidget(parent)
	, maximum_(0)
	, minimum_(0)
	, page_step_(1)
	, single_step_(1)
	, position_(0)
	, value_(0)
	, slider_down_(true)
	, tracking_(true)
	, orientation_(Qt::Horizontal)
{
}

AbstractSlider::AbstractSlider(Qt::Orientation orientation, QWidget *parent)
	: QWidget(parent)
	, maximum_(0)
	, minimum_(0)
	, page_step_(1)
	, single_step_(1)
	, position_(0)
	, value_(0)
	, slider_down_(true)
	, tracking_(true)
	, orientation_(orientation)
{
}

void AbstractSlider::sliderChange(SliderChange)
{
}

