
#include <QtGui>
#include "view.h"
#include "document.h"

View::View(QWidget *parent, Document *doc)
	: QWidget(parent)
	, doc_(doc)
	, cur_(NULL)
{
}


