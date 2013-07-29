
#pragma once

#include <QWidget>
#include "global.h"

class Document;

namespace Standard {

	class Global;
	class AddressView;

	class Editor : public QWidget
	{
		Q_OBJECT
	public:
		Editor(QWidget *parent = NULL);
		Editor(QWidget *parent, ::Document *document);

		::Document *document() const
		{
			return global_->document();
		}

	protected:

		void initView();

		void resizeEvent(QResizeEvent*);
		void paintEvent(QPaintEvent *ev);
		void focusInEvent(QFocusEvent*);

	protected:

		Global *global_;
		AddressView *view_;

	};


}	// namespace Standard
