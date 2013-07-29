
#pragma once

#include "abstractslider.h"


class ScrollBar : public AbstractSlider
{
	Q_OBJECT

public:
	ScrollBar(QWidget *parent = NULL);
	ScrollBar(Qt::Orientation orientation, QWidget *parent = NULL);
	~ScrollBar();


	void mouseMoveEvent(QMouseEvent *ev);
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);


	void paintEvent(QPaintEvent *ev);

	QSize sizeHint() const;

protected:
	int clientSize() const;
	void sliderChange(SliderChange);

	bool drawScrollBar() const;
	int drawBarSize() const;

	bool isScrollBarArea(const QPoint &pos) const;


	// logical value to pixel position
	int sliderPositionFromValue(qint64 logicalValue, int span, bool upsideDown = false) const;
	// pixel position to logical value
	qint64 sliderValueFromPosition(int position, int span, bool upsideDown = false);

protected:
	int base_size_;
	int absolute_bar_min_;
	int blank_size_;
	int bar_grabed_position_;
	bool mouse_grabed_;

};


