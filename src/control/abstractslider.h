
#pragma once

#include <QDebug>
#include <QWidget>

class AbstractSlider : public QWidget
{
	Q_OBJECT

	//Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)
	//Q_PROPERTY(bool invertedControls READ invertedControls WRITE setInvertedControls)
	Q_PROPERTY(qint64 maximum READ maximum WRITE setMaximum)
	Q_PROPERTY(qint64 minimum READ minimum WRITE setMinimum)
	Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
	Q_PROPERTY(qint64 pageStep READ pageStep WRITE setPageStep)

public:
	AbstractSlider(QWidget *parent = NULL);
	AbstractSlider(Qt::Orientation, QWidget *parent = NULL);

	bool hasTracking() const
	{
		return tracking_;
	}

	void setTracking(bool enable)
	{
		tracking_ = enable;
	}


	//bool invertedAppearance() const;
	//bool invertedControls() const;

	bool isSliderDown() const
	{
		return slider_down_;
	}
	//bool isSliderDown() const;
	//
	qint64 maximum() const
	{
		return maximum_;
	}

	qint64 minimum() const
	{
		return minimum_;
	}

	Qt::Orientation orientation() const
	{
		return orientation_;
	}

	qint64 pageStep() const
	{
		return page_step_;
	}

	//void setInvertedAppearance(bool);
	//void setInvertedControls(bool);

	void setMaximum(qint64 max)
	{
		if (maximum_ != max) {
			maximum_ = max;
			emit rangeChanged(minimum_, maximum_);
		}
	}
	void setMinimum(qint64 min)
	{
		if (minimum_ != min) {
			minimum_ = min;
			emit rangeChanged(minimum_, maximum_);
		}
	}

	void setPageStep(qint64 step)
	{
		page_step_ = step;
	}

	void setSingleStep(qint64 step)
	{
		single_step_ = step;
	}

	void setSliderDown(bool down)
	{
		slider_down_ = down;
	}

	void setSliderPosition(qint64 position)
	{
		if (position_ != position) {
			position_ = position;
			emit sliderMoved(position);
		}
	}

	qint64 singleStep() const
	{
		return single_step_;
	}

	qint64 sliderPosition() const
	{
		return position_;
	}
	//void triggerAction(SliderAction action)
	//{
	//	action_ = action;
	//}
	qint64 value() const
	{
		return value_;
	}


public slots:
	void setOrientation(Qt::Orientation orientation)
	{
		if (orientation_ != orientation) {
			orientation_ = orientation;
			sliderChange(SliderOrientationChange);
		}
	}

	void setRange(qint64 min, qint64 max)
	{
		if (minimum_ != min || maximum_ != max) {
			qDebug() << "setRange " << min << " " << max;
			minimum_ = min;
			maximum_ = max;
			emit rangeChanged(minimum_, maximum_);
			sliderChange(SliderRangeChange);
		}
	}

	void setValue(qint64 pos)
	{
		if (value_ != pos) {
			qDebug() << "setValue " << pos;
			value_ = pos;
			emit valueChanged(pos);
			sliderChange(SliderValueChange);
		}
	}

	// compatible for QAbstractSlider slots
	void setRange(int min, int max)
	{
		setRange(static_cast<qint64>(min), static_cast<qint64>(max));
	}

	void setValue(int pos)
	{
		setValue(static_cast<qint64>(pos));
	}

public:
	enum SliderAction {
		SliderNoAction,
		SliderSingleStepAdd,
		SliderSingleStepSub,
		SliderPageStepAdd,
		SliderPageStepSub,
		SliderToMinimum,
		SliderToMaximum,
		SliderMove
	};

protected:

	enum SliderChange {
		SliderRangeChange,
		SliderOrientationChange,
		SliderStepsChange,
		SliderValueChange
	};

	//SliderAction repeatAction() const;
	//void setRepeatAction(SliderAction action, int thresholdTime = 500, int repeatTime = 50);
	virtual void sliderChange(SliderChange change);

signals:
	void rangeChanged(qint64 min, qint64 max);
	void actionTriggered(int action);
	void sliderMoved(qint64 value);
	void sliderPressed();
	void sliderReleased();
	void valueChanged(qint64 value);

protected:
	qint64 maximum_;
	qint64 minimum_;
	qint64 page_step_;
	qint64 single_step_;
	qint64 position_;
	qint64 value_;
	bool slider_down_;
	bool tracking_;
	Qt::Orientation orientation_;

};

