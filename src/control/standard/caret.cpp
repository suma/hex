
#include "caret.h"

namespace Standard {

Caret::Caret(CaretShape visible, CaretShape invisible)
	: visible_shape_(visible)
	, invisible_shape_(invisible)
	, timer_id_(0)
	, is_visible_(true)
{
}

Caret::~Caret()
{
}


}	// namespace

