#include <QApplication>
#include <QtDebug>
#include "../control/document.h"

#define DO(a) puts("exec: " #a); (a);
int main(int argc, char *argv[])
{
	printf("test\n");

	Document doc;
	uchar buf[1024];
	DO( doc.insert(0, (uchar*)"abcdef", 6) );
	printf("size: %lld\n", doc.length());

	DO( doc.remove(2,4) );
	//for (int i = 0; i < 1000000; i++) 
	DO( doc.insert(2, (uchar*)"01$#", 4) );
	printf("size: %lld\n", doc.length());
	DO( doc.get(2, buf, 2) );
	buf[2]=0;
	printf("doc[23] = %s\n", buf);


	//QApplication app(argc, argv);
	//Standard::HexView *hview = new Standard::HexView();
	//hview->show();
	//QLabel *label = new QLabel("Hello Qt!");
	//label->show();
	//Editor *editor = new Editor();
	//editor->show();
	//
	//return app.exec();
	return 0;
}

