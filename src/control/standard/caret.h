
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

		CaretShape getVisibleShape() const
		{
			return visible_shape_;
		}

		CaretShape getInvisibleShape() const
		{
			return invisible_shape_;
		}

		CaretShape getCurrentShape() const
		{
			return getShape(is_visible_);
		}

		CaretShape getShape(bool visible) const
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

		int  getTimerId() const
		{
			return timer_id_;
		}

		bool getVisible() const
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

