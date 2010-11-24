
#pragma once

#include <QWidget>
#include "color.h"

class Document;

namespace Standard {

	class View : public QWidget
	{
		Q_OBJECT

	protected:
		Document *document_;
		QPixmap pix_;


	public:
		View(QWidget *parent = NULL, Document *doc = NULL);
		virtual ~View();

	private:
		void paintEvent(QPaintEvent*);

	protected:
		void resizeEvent(QResizeEvent*);
		virtual void caretDrawEvent(QPainter *) = 0;
	};



}	// namespace

