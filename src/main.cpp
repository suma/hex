#include <QApplication>
#include "mywidgets.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	Mywidgets wid;
	wid.show();
	return app.exec();
}

