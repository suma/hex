#include <QApplication>
#include <QTextCodec>
#include "mainform.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
	MainForm *editor = new MainForm();
	editor->show();
	return app.exec();
}

