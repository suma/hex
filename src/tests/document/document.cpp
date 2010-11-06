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
	void cleanupTestCase();
	void openFile();

private:
	QFile* open();

private:
	QTemporaryFile *temp_file_;
	std::vector<char> data_;
};

const int SIZE = 1024 * 1024 * 2;	// 64MB

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

	QFile *file = open();
	QVERIFY(file->seek(0));

	QDataStream outStream(file);
	QVERIFY(outStream.writeRawData(&data_[0], SIZE) == SIZE);
	//QVERIFY(file_->writeData(&data_[0], SIZE) == SIZE);
}

void TestDocument::cleanupTestCase()
{
	// delete dummy file
	delete temp_file_;

}

QFile* TestDocument::open()
{
	QFile *file = new QFile(temp_file_->fileName());
	file->open(QIODevice::ReadWrite);
	return file;
}




// TEST


void TestDocument::openFile()
{
	QFile *file = open();
	Document *doc = new Document(file);

	// check length
	QVERIFY(file->size() == doc->length());

	// verify data
	const quint64 COPY_SIZE = 1000;
	uchar buff[COPY_SIZE];
	for (quint64 pos = 0; pos < doc->length(); pos += COPY_SIZE) {
		uint copy_size = (uint)qMin((quint64)COPY_SIZE, doc->length() - (quint64)pos);
		doc->get(pos, buff, copy_size);
		QVERIFY(memcmp(buff, &data_[pos], copy_size) == 0);
	}
	

	delete doc;
}

QTEST_MAIN(TestDocument)
#include "document.moc"


