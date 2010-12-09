
#include "caret.h"

namespace Standard {

Caret::Caret(CaretShape visible, CaretShape invisible)
	: visible_shape_(visible)
	, invisible_shape_(invisible)
	, timer_id_(0)
	, blink_time_(500)
	, enable_(true)
	, is_visible_(true)
{
}

Caret::~Caret()
{
}


}	// namespace

