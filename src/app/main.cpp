#include <QApplication>
#include <QTextCodec>
#include "editor.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
	qDebug("main()");
	//Standard::HexView *hview = new Standard::HexView();
	//hview->show();
	//QLabel *label = new QLabel("Hello Qt!");
	//label->show();
	Editor *editor = new Editor();
	editor->show();
	return app.exec();
}

