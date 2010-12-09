
#pragma once

#include "layeredwidget.h"

namespace Standard {


	class View : public LayeredWidget
	{
		Q_OBJECT

	public:
		View(QWidget *parent = NULL);
		~View();

	private:
		void focusInEvent(QFocusEvent*);
		void focusOutEvent(QFocusEvent*);

	signals:
		void focusIn();
		void focusOut();

	};



}	// namespace

