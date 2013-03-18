#include <QTextCodec>
#include "hexapp.h"
#include "mainform.h"

int main(int argc, char *argv[])
{
	HexApp app(argc, argv);
	QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
	MainForm *editor = new MainForm();
	editor->show();
	return app.exec();
}

