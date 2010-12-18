

#include "view.h"

namespace Standard {

View::View(QWidget *parent)
	: LayeredWidget(parent)
{
}

View::~View()
{
}


void View::focusInEvent(QFocusEvent*)
{
	emit focusIn();
}

void View::focusOutEvent(QFocusEvent*)
{
	emit focusOut();
}



}	// namespace

