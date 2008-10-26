#ifndef VIEW_H_INC
#define VIEW_H_INC

#include <QWidget>

class Document;
class Cursor;

class View : public QWidget
{
	Q_OBJECT

protected:
	Document *doc_;
	Cursor *cur_;

public:
	View(QWidget *parent = 0, Document *doc = 0);


};



#endif
