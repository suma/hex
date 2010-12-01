
#include "view.h"
#include "caretdrawer.h"
#include "../document.h"

namespace Standard {

View::View(QWidget *parent, Document *doc)
	: LayeredWidget(parent)
	, document_(doc)
{
}

View::~View()
{
}


}	// namespace

