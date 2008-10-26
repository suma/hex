#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	//List wid;
	//wid.show();
	QLabel *label = new QLabel("Hello Qt!");
	label->show();
	return app.exec();
}

