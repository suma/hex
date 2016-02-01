#include <QMouseEvent>
#include <QPainter>
#include "scrollbar.h"


ScrollBar::ScrollBar(QWidget *parent)
  : AbstractSlider(parent)
  , base_size_(30)
  , absolute_bar_min_(5)
  , blank_size_(5)
  , bar_grabed_position_(0)
  , mouse_grabed_(false)
{
  setMouseTracking(true);
}

ScrollBar::ScrollBar(Qt::Orientation orientation, QWidget *parent)
  : AbstractSlider(orientation, parent)
  , base_size_(30)
  , absolute_bar_min_(5)
  , blank_size_(5)
  , bar_grabed_position_(0)
  , mouse_grabed_(false)
{
  setMouseTracking(true);
}

ScrollBar::~ScrollBar()
{
}

void ScrollBar::mouseMoveEvent(QMouseEvent *ev)
{
  if (mouse_grabed_) {
    // position diff
    int pos = (orientation_ == Qt::Horizontal ? ev->pos().x() : ev->pos().y()) - bar_grabed_position_;

    // fix negative
    pos = qMax(pos, 0);
    Q_ASSERT(pos >= 0);

    // fix maximum
    const qint64 value = qMin(sliderValueFromPosition(pos, drawBarSize()), maximum_);
    Q_ASSERT(value <= maximum_);

    if (isSliderDown()) {
      setSliderPosition(value);
    }

    if (hasTracking()) {
      setValue(value);
    }
  } else {
    // When mouse hovering
    if (isScrollBarArea(ev->pos())) {
      // TODO: animation
    }
  }
}

void ScrollBar::mousePressEvent(QMouseEvent *ev)
{
  if (isScrollBarArea(ev->pos())) {
    // drag
    const int pos = (orientation_ == Qt::Horizontal ? ev->pos().x() : ev->pos().y());
    bar_grabed_position_ = pos - sliderPositionFromValue(value_, drawBarSize());
    mouse_grabed_ = true;
    emit sliderPressed();
    grabMouse();
  } else {
    // pageup/pagedown
  }
}

void ScrollBar::mouseReleaseEvent(QMouseEvent *)
{
  mouse_grabed_ = false;
  emit sliderReleased();
  releaseMouse();
}

bool ScrollBar::drawScrollBar() const
{
  return drawBarSize() != 0;
}

int ScrollBar::clientSize() const
{
  return qMax((orientation_ == Qt::Horizontal ? width() : height()) - blank_size_, 0);
}

int ScrollBar::drawBarSize() const
{
  const int bar_size = base_size_ / 2;

  const qint64 scroll_len = maximum_ - minimum_ + page_step_;

  const double ratio = (double)page_step_ / scroll_len;
  if (ratio <= 0 || ratio >= 1) {
    return 0;
  }

  return qMax(static_cast<int>(clientSize() * ratio), bar_size);
}

int ScrollBar::sliderPositionFromValue(qint64 logicalValue, int span, bool upsideDown) const
{
  // fix client size using barsize
  const int fix_client_size = qMax(clientSize() - span, 0);

  const qint64 s_len = maximum_ - minimum_;
  const qint64 s_pos = maximum_ - minimum_ - logicalValue;

  // calc ratio
  const double screen_pos_ratio = (double)1 - ((double)s_pos / (double)s_len);

  // value to position
  return qMin(static_cast<int>(fix_client_size * screen_pos_ratio), fix_client_size);
}

qint64 ScrollBar::sliderValueFromPosition(int position, int span, bool upsideDown)
{
  // fix client size using barsize
  const int fix_client_size = qMax(clientSize() - span, 0);

  const qint64 s_len = maximum_ - minimum_;
  const double value_ratio = (double)position / (double)fix_client_size;

  // position to value
  return static_cast<qint64>(s_len * value_ratio);
}

void ScrollBar::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.fillRect(rect(), QBrush(QColor(200,200,255), Qt::BDiagPattern));

  if (!drawScrollBar()) {
    return;
  }

  // bar width(height)
  const int bar_size = base_size_ / 2;

  const int draw_bar_size = drawBarSize();
  const int pos = sliderPositionFromValue(value_, draw_bar_size);

  if (orientation_ == Qt::Horizontal) {
    // absolute bar(down side)

    // relative bar(upper side)

  }

  if (orientation_ == Qt::Vertical) {
    //qDebug() << "paint pos:" << pos << " size:" << draw_bar_size;
    // absolute bar(right side)
    painter.fillRect(bar_size, pos, bar_size, draw_bar_size, QBrush(QColor(255, 255, 0, 160)));

    // relative bar(left side)
  }
}

bool ScrollBar::isScrollBarArea(const QPoint &mouse_pos) const
{
  if (!drawScrollBar()) {
    return false;
  }

  const int bar_size = base_size_ / 2;
  const int draw_bar_size = drawBarSize();
  const int pos = sliderPositionFromValue(value_, draw_bar_size);

  if (orientation_ == Qt::Horizontal) {
    //return QRect(bar_size, pos, bar_size, draw_bar_size).contains.contains(pos);
  }

  if (orientation_ == Qt::Vertical) {
    return QRect(bar_size, pos, bar_size, draw_bar_size).contains(mouse_pos);
  }

  return false;
}


QSize ScrollBar::sizeHint() const
{
  if (orientation_ == Qt::Horizontal) {
    return QSize(width(), base_size_);
  } else {
    // Vertical
    return QSize(base_size_, height());
  }
}

void ScrollBar::sliderChange(SliderChange change)
{
  switch (change) {
  case SliderRangeChange:
    repaint();  // redraw all
    break;
  case SliderOrientationChange:
    repaint();  // redraw all
    break;
  case SliderStepsChange:
    // TODO: not implmented in AbstractSlider
    break;
  case SliderValueChange:
    repaint();  // redraw all
    break;
  }
}
