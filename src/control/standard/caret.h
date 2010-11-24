
#pragma once

#include "cursorutil.h"

namespace Standard {

	class Caret
	{
	private:
		CaretShape visible_shape_;
		CaretShape invisible_shape_;
		int timer_id_;
		bool is_visible_;

	public:
		Caret(CaretShape visible = CARET_BLOCK, CaretShape invisible = CARET_FRAME);
		~Caret();

		void setVisibleShape(CaretShape shape)
		{
			visible_shape_ = shape;
		}

		void setInvisibleShape(CaretShape shape)
		{
			invisible_shape_ = shape;
		}

		CaretShape visibleShape() const
		{
			return visible_shape_;
		}

		CaretShape invisibleShape() const
		{
			return invisible_shape_;
		}

		CaretShape getCurrentShape() const
		{
			return shape(is_visible_);
		}

		CaretShape shape(bool visible) const
		{
			if (visible) {
				return visible_shape_;
			} else {
				return invisible_shape_;
			}
		}

		void setTimerId(int id)
		{
			timer_id_ = id;
		}

		int  timerId() const
		{
			return timer_id_;
		}

		bool visible() const
		{
			return is_visible_;
		}

		void setVisible(bool t)
		{
			is_visible_ = t;
		}

		void inverseVisible()
		{
			is_visible_ = !is_visible_;
		}
	};


}	// namespace

