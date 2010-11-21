#include <QtTest/QtTest>
#include <QTemporaryFile>
#include <QDataStream>
#include <vector>
#include "control/document.h"
#include "control/commands.h"

class TestDocument: public QObject
{
	Q_OBJECT
public:
	TestDocument();

private slots:
	void initTestCase();
	void cleanupTestCase();
	void testOpenFile();
	void testUndoDelete();
	void testUndoInsert();
	void testUndoReplace();

private:
	QFile *openFile();
	Document *open();

private:
	Document *doc_;
	QFile *file_;
	QTemporaryFile *temp_file_;
	std::vector<char> data_;
};

const int SIZE = 1024 * 1024 * 2;	// 64MB
const uint MAP_BLOCK_SIZE = 0x1000;


TestDocument::TestDocument()
	: doc_(NULL)
	, file_(NULL)
	, temp_file_(NULL)
{
}

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

	openFile();
	QVERIFY(file_->seek(0));

	QDataStream outStream(file_);
	QVERIFY(outStream.writeRawData(&data_[0], SIZE) == SIZE);
}

void TestDocument::cleanupTestCase()
{
	delete doc_;	// 同時にfile_も削除される
	doc_ = NULL;

	// delete dummy file
	delete temp_file_;
	temp_file_ = NULL;

}

QFile* TestDocument::openFile()
{
	file_ = new QFile(temp_file_->fileName());
	file_->open(QIODevice::ReadWrite);
	return file_;
}


Document *TestDocument::open()
{
	openFile();
	doc_ = new Document(file_, MAP_BLOCK_SIZE);
	return doc_;
}


// TEST


void TestDocument::testOpenFile()
{
	Document *doc = open();

	// check length
	QVERIFY(file_->size() == doc->length());

	// verify data
	const quint64 COPY_SIZE = 1000;

	QVERIFY( COPY_SIZE <= MAP_BLOCK_SIZE );

	uchar buff[COPY_SIZE];
	for (quint64 pos = 0; pos < doc->length(); pos += COPY_SIZE) {
		uint copy_size = (uint)qMin((quint64)COPY_SIZE, doc->length() - (quint64)pos);
		doc->get(pos, buff, copy_size);
		QVERIFY(memcmp(buff, &data_[pos], copy_size) == 0);
	}
	
}

void TestDocument::testUndoDelete()
{
	Document *doc = open();

	// check check length
	QVERIFY(file_->size() == doc->length());

	size_t DELETE_SIZE = 100;
	for (int i = 0; i < DELETE_SIZE; i++) {
		doc->undoStack()->push(new DeleteCommand(doc, i, 1));
	}

	// after delete
	QVERIFY(file_->size() == (doc->length() + DELETE_SIZE));

	while (doc->undoStack()->canUndo()) {
		doc->undoStack()->undo();
	}

	// 元のサイズ
	QVERIFY(file_->size() == doc->length());
}

void TestDocument::testUndoInsert()
{
	Document *doc = open();

	// check check length(before insert)
	QVERIFY(file_->size() == doc->length());

	// Insert data
	size_t INSERT_SIZE = 1000;
	uchar data = 0x90;
	for (int i = 0; i < INSERT_SIZE; i++) {
		doc->undoStack()->push(new InsertCommand(doc, doc->length() / 2 + i, &data, 1));
	}

	// after insert
	QVERIFY(file_->size() == (doc->length() - INSERT_SIZE));

	// Undo
	while (doc->undoStack()->canUndo()) {
		doc->undoStack()->undo();
	}

	// 元のサイズ
	QVERIFY(file_->size() == doc->length());
}

void TestDocument::testUndoReplace()
{
	Document *doc = open();

	// check check length
	QVERIFY(file_->size() == doc->length());

	// Replace data
	size_t REPLACE_SIZE = 3;
	uchar data = 0x90;
	for (int i = 0; i < REPLACE_SIZE; i++) {
		doc->undoStack()->push(new ReplaceCommand(doc, i, 1, &data, 1));
	}

	
	// after replace
	QVERIFY(file_->size() == doc->length());
	
	// Undo
	while (doc->undoStack()->canUndo()) {
		doc->undoStack()->undo();
	}

	// 元のサイズ
	QVERIFY(file_->size() == doc->length());
}

QTEST_MAIN(TestDocument)
#include "document.moc"


