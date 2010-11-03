#include <QtTest/QtTest>
#include <QTemporaryFile>
#include <QDataStream>
#include <vector>
#include "control/document.h"

class TestDocument: public QObject
{
	Q_OBJECT
private slots:
	void initTestCase();
	void openFile();
	void cleanupTestCase();
	void open();
private:
	QTemporaryFile *temp_file_;
	QFile *file_;
	std::vector<char> data_;
};

const int SIZE = 1024 * 1024 * 16;	// 64MB

void TestDocument::initTestCase()
{
	// write to dummy file
	data_.resize(SIZE);
	qsrand(107);
	for (int i = 0; i < SIZE; i++) {
		data_.push_back(static_cast<char>(qrand() & 0xFF));
	}

	temp_file_ = new QTemporaryFile();
	QVERIFY2(temp_file_->open(), "file open failed");

	temp_file_->setAutoRemove(true);

	open();
	QVERIFY(file_->seek(0));

	QDataStream outStream(file_);
	QVERIFY(outStream.writeRawData(&data_[0], SIZE) == SIZE);
	//QVERIFY(file_->writeData(&data_[0], SIZE) == SIZE);
}

void TestDocument::cleanupTestCase()
{
	// delete dummy file
	delete temp_file_;

}

void TestDocument::open()
{
	file_ = new QFile(temp_file_->fileName());
	QVERIFY2(file_->open(QIODevice::ReadWrite), "file open failed");
}

void TestDocument::openFile()
{
	QFile *file = new QFile(QString("./tests"));
	Document *doc = new Document(file);

	// copy data
	
	// verify data

	delete doc;
}

QTEST_MAIN(TestDocument)
#include "document.moc"


