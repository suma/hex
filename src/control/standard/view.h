
#pragma once

#include "layeredwidget.h"
#include "color.h"

class Document;

namespace Standard {

	class View : public LayeredWidget
	{
		Q_OBJECT

	protected:
		Document *document_;
		QPixmap pix_;


	public:
		View(QWidget *parent = NULL, Document *doc = NULL);
		virtual ~View();

	};



}	// namespace

