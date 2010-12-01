

#include "layeredwidget.h"

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

	if (layers_.lastIndexOf(widget) > 0) {
		return;
	}

	widget->setParent(this);
	widget->move(0, 0);
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

	if (event->type() == QEvent::ChildAdded) {
		// add
		add(reinterpret_cast<QWidget*>(child));
	} else if (event->type() == QEvent::ChildRemoved) {
		// removed
		remove(reinterpret_cast<QWidget*>(child));
	}
}

void LayeredWidget::resizeEvent(QResizeEvent *)
{
	for (int i = 0; i < layers_.size(); i++) {
		layers_[i]->resize(width(), height());
	}
}


}	// namespace

