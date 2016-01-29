#include "layeredwidget.h"
#include <QChildEvent>
#include <QResizeEvent>

namespace Standard {

LayeredWidget::LayeredWidget(QWidget *widget)
  : QWidget(widget)
{
}

LayeredWidget::~LayeredWidget()
{
}

void LayeredWidget::add(QWidget *widget)
{
  Q_ASSERT(widget != NULL);

  if (layers_.lastIndexOf(widget) != -1) {
    return;
  }

  widget->setParent(this);
  widget->move(0, 0);
  layers_.append(widget);
}

void LayeredWidget::remove(QWidget *widget)
{
  Q_ASSERT(widget != NULL);

  const int index = layers_.lastIndexOf(widget);
  if (index == -1) {
    return;
  }

  layers_.erase(layers_.begin() + index);
  // TODO: must not delete pointer here
}

void LayeredWidget::childEvent(QChildEvent *event)
{
  QObject *child = event->child();
  if (!child->isWidgetType()) {
    return;
  }

  QWidget *widget = dynamic_cast<QWidget*>(child);
  if (event->type() == QEvent::ChildAdded) {
    // add
    add(widget);
  } else if (event->type() == QEvent::ChildRemoved) {
    // removed
    remove(widget);
  }
}

void LayeredWidget::resizeEvent(QResizeEvent *event)
{
  for (int i = 0; i < layers_.size(); i++) {
    layers_[i]->resize(event->size().width(), event->size().height());
  }
}

}  // namespace
