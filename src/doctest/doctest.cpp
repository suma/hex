#include <QApplication>
#include <QtTest/QtTest>
#include <QtDebug>
#include "../control/document.h"

/*
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
*/

#include "rectest.h"

class DocTest : public QObject
{
  Q_OBJECT
private slots:

  void test1();
  void rec1();

};

void DocTest::test1()
{

#define DO(a) puts("exec: " #a); (a);

  Document doc;
  uchar buf[1024];

  QCOMPARE( doc.length(), quint64(0) );

  DO( doc.insert(0, (uchar*)"abcdef", 6) );

  QCOMPARE( doc.length(), quint64(6) );

  DO( doc.remove(2,4) );
  QCOMPARE( doc.length(), quint64(2) );

  DO( doc.insert(2, (uchar*)"01$#", 4) );

  QCOMPARE( doc.length(), quint64(6) );

  DO( doc.get(2, buf, 2) );
}

void DocTest::rec1()
{
  srand(127);
  Test t("aa");
  for ( int i = 0; i < TEST_MAX+1; i++ ) {
    t.TestUnit();
  }
}


QTEST_MAIN(DocTest)
#include "doctest.moc"
