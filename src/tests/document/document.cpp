#include <QtTest/QtTest>
#include "control/document.h"

class TestDocument: public QObject
{
	Q_OBJECT
private slots:
	void toUpper();
};

void TestDocument::toUpper()
{
	Document *doc = new Document();
	QString str = "Hello";
	QCOMPARE(str.toUpper(), QString("HELLO"));
}

QTEST_MAIN(TestDocument)
#include "document.moc"


