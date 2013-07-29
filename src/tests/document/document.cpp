#include <QtTest/QtTest>
#include <QTemporaryFile>
#include <QDataStream>
#include <vector>
#include "control/document.h"
#include "control/commands.h"
#include "control/filemapreader.h"

class TestDocument: public QObject
{
	Q_OBJECT
public:
	TestDocument();

private slots:
	void initTestCase();
	void cleanupTestCase();

	void checkCompare();

	// FileMapReader
	void testOpenReader();

	// Document
	void testOverwritable1();
	void testOverwritable2();
	void testOpenFile();
	void testUndoDelete();
	void testUndoInsert();
	void testUndoReplace();
	void testUndoStack();

	void testDocumentSaveAs();
	void testDocumentSave();

private:
	QTemporaryFile *createTemp();
	QFile *openFile();
	Document *open();
	static Document *createDoc(QFile *file);

	bool compareDocument(QFile *file, Document *doc);
	bool compareDocument(Document *, Document *);

	void dumpFragments(Document *doc);

private:
	QTemporaryFile *temp_file_;
	std::vector<uchar> data_;
};

const int TEST_FILE_SIZE = 1024 * 1024 * 2;	// 64MB
const uint MAP_BLOCK_SIZE = 0x1000;
const uint BLOCK_SIZE = 0x1000;

const int WRITE_TEST_REPEAT = 10;

TestDocument::TestDocument()
	: temp_file_(NULL)
{
}

QTemporaryFile *TestDocument::createTemp()
{
	// write to dummy file
	data_.resize(TEST_FILE_SIZE);
	qsrand(107);
	for (int i = 0; i < TEST_FILE_SIZE; i++) {
		data_.push_back(static_cast<uchar>(qrand() & 0xFF));
	}

	QTemporaryFile *temp = new QTemporaryFile();
	temp->setAutoRemove(true);
	Q_ASSERT(temp->open());
	QDataStream outStream(temp);
	Q_ASSERT(outStream.writeRawData(reinterpret_cast<char*>(&data_[0]), TEST_FILE_SIZE) == TEST_FILE_SIZE);

	return temp;
}

void TestDocument::initTestCase()
{
	temp_file_ = createTemp();
}

void TestDocument::cleanupTestCase()
{
	// delete dummy file
	delete temp_file_;
	temp_file_ = NULL;

}

QFile* TestDocument::openFile()
{
	QFile *file = new QFile(temp_file_->fileName());
	Q_ASSERT(file->open(QIODevice::ReadWrite));
	return file;
}


Document *TestDocument::open()
{
	QFile *file = openFile();
	return new Document(file, MAP_BLOCK_SIZE);
}

Document *TestDocument::createDoc(QFile *file)
{
	return new Document(file, MAP_BLOCK_SIZE);
}

bool TestDocument::compareDocument(QFile *file, Document *doc)
{
	Q_ASSERT(file != NULL && doc != NULL);

	// length
	if (static_cast<quint64>(file->size()) != doc->length()) {
		return false;
	}

	// file data
	QDataStream stream(file);
	file->seek(0);

	uchar buffer1[BLOCK_SIZE];
	char buffer2[BLOCK_SIZE];

	for (quint64 pos = 0, len = doc->length(); len != 0; ) {
		const uint copy_size = static_cast<uint>(qMin(len, (quint64)BLOCK_SIZE));
		doc->get(pos, buffer1, copy_size);
		int res = stream.readRawData(buffer2, copy_size);
		Q_ASSERT(res != -1);

		if (memcmp(buffer1, buffer2, copy_size)) {
			return false;
		}

		pos += copy_size;
		len -= copy_size;
	}
	
	return true;
}


bool TestDocument::compareDocument(Document *lhs, Document *rhs)
{
	Q_ASSERT(lhs != NULL && rhs != NULL);

	if (lhs->length() != rhs->length()) {
		return false;
	}

	uchar buffer1[BLOCK_SIZE];
	uchar buffer2[BLOCK_SIZE];

	for (quint64 pos = 0, len = lhs->length(); len != 0; ) {
		const uint copy_size = static_cast<uint>(qMin(len, (quint64)BLOCK_SIZE));
		lhs->get(pos, buffer1, copy_size);
		rhs->get(pos, buffer2, copy_size);

		if (memcmp(buffer1, buffer2, copy_size)) {
			return false;
		}

		pos += copy_size;
		len -= copy_size;
	}

	return true;
}

void TestDocument::dumpFragments(Document *doc)
{
	Q_ASSERT(doc != NULL);

	Document::FragmentList fragments(doc->get());
	Document::FragmentList::iterator it = fragments.begin();
	qDebug() << doc << "length =" << doc->length() << " fragment.size() =" << fragments.size();
	quint64 index = 0;
	while (it != fragments.end()) {
		if (it->type() == Document::DOCTYPE_ORIGINAL) {
			qDebug() << "index = " << index <<
				"(type, pos, len) =" << it->type() << it->position() << it->length();
			if (index != it->position()) {
				qDebug() << "not equals index and position";
			}
		}
		index += it->length();
		++it;
	}
	qDebug() << "<END>";
}



// TEST

void TestDocument::checkCompare()
{
	{
		Document *doc = open();
		QVERIFY(compareDocument(doc->file(), doc));
		delete doc;
	}

	{
		Document *lhs = open();
		Document *rhs = open();
		QVERIFY(compareDocument(lhs, rhs));
		delete lhs;
		delete rhs;
	}

	{
		QTemporaryFile *file = createTemp();
		QDataStream data(file);

		// append string
		data << "123456";
		QVERIFY(file->size() != temp_file_->size());

		// compare
		Document *doc = open();
		Document *doc2 = createDoc(file);
		QVERIFY(compareDocument(file, doc) == false);
		QVERIFY(compareDocument(doc, doc2) == false);
		delete doc;
		delete doc2;	// delete file in doc2
	}

	{
		QTemporaryFile *file = createTemp();
		QDataStream data(file);

		// rewrite first 4bytes
		const char *test = "1234";
		file->seek(0);
		QVERIFY(data.writeRawData(test, 4) == 4);
		QVERIFY(file->size() == temp_file_->size());

		// compare
		Document *doc = open();
		Document *doc2 = createDoc(file);
		QVERIFY(compareDocument(file, doc) == false);
		QVERIFY(compareDocument(doc, doc2) == false);
		delete doc;
		delete doc2;	// delete file in doc2
	}
}

void TestDocument::testOverwritable1()
{
	Document *doc = open();
	QVERIFY(doc->overwritable() == true);
	delete doc;
}

void TestDocument::testOverwritable2()
{
	Document *doc = open();

	// Replace data
	std::vector<uchar> dummy;
	const int SIZE = 1024 * 100;
	int insert_size = SIZE;
	for (int insert_pos = doc->length() / 2; insert_size > 0; ) {
		// assert: pos + len <= doc->lenght
		const int size = qMin(qMin(1024, qrand()), insert_size);
		// fix insert_pos
		while (insert_pos + size > doc->length()) {
			insert_pos -= qMin(insert_pos, qrand());
		}
		dummy.resize(size);
		doc->undoStack()->push(new ReplaceCommand(doc, insert_pos, size, &dummy[0], size));


		//dumpFragments(doc);

		// ovewritable
		QVERIFY(doc->overwritable());

		insert_pos = (insert_pos + size + qrand()) % doc->length();
		insert_size -= size;
	}

	delete doc;
}

void TestDocument::testOpenReader()
{
	QFile *file = openFile();
	Q_ASSERT(file != NULL);

	const quint64 max = file->size();
	const size_t COPY_SIZE = 1000;
	{
		FileMapReader reader(file, COPY_SIZE);

		QVERIFY(reader.seek(0));

		for (quint64 pos = 0; pos < max; pos += COPY_SIZE) {
			const uint copy_size = (uint)qMin((quint64)COPY_SIZE, file->size() - pos);

			const uchar *ptr = reader.get();
			Q_ASSERT(ptr != NULL);

			QVERIFY(memcmp(ptr, &data_[pos], copy_size) == 0);
			reader += COPY_SIZE;
		}
	}

	delete file;
}


void TestDocument::testOpenFile()
{
	Document *doc = open();
	QFile *file = doc->file();

	// check length
	QVERIFY(file->size() == doc->length());

	// verify data
	const quint64 COPY_SIZE = 1000;

	QVERIFY( COPY_SIZE <= MAP_BLOCK_SIZE );

	uchar buff[COPY_SIZE];
	for (quint64 pos = 0; pos < doc->length(); pos += COPY_SIZE) {
		uint copy_size = (uint)qMin((quint64)COPY_SIZE, doc->length() - pos);
		doc->get(pos, buff, copy_size);
		QVERIFY(memcmp(buff, &data_[pos], copy_size) == 0);
	}
	
	delete doc;
}

void TestDocument::testUndoDelete()
{
	Document *doc = open();
	QFile *file = doc->file();

	// check check length
	QVERIFY(file->size() == doc->length());

	size_t DELETE_SIZE = 100;
	for (size_t i = 0; i < DELETE_SIZE; i++) {
		doc->undoStack()->push(new DeleteCommand(doc, i, 1));
	}

	// after delete
	QVERIFY(file->size() == (doc->length() + DELETE_SIZE));

	while (doc->undoStack()->canUndo()) {
		doc->undoStack()->undo();
	}

	// Compare original
	Document *orig = open();
	QVERIFY(file->size() == doc->length());
	QVERIFY(compareDocument(doc, orig));
	delete orig;

	delete doc;
}

void TestDocument::testUndoInsert()
{
	Document *doc = open();
	QFile *file = doc->file();

	// check check length(before insert)
	QVERIFY(file->size() == doc->length());

	// Insert data
	size_t INSERT_SIZE = 1000;
	uchar data = 0x90;
	for (size_t i = 0; i < INSERT_SIZE; i++) {
		doc->undoStack()->push(new InsertCommand(doc, doc->length() / 2 + i, &data, 1));
	}

	// after insert
	QVERIFY(file->size() == (doc->length() - INSERT_SIZE));

	// Undo
	while (doc->undoStack()->canUndo()) {
		doc->undoStack()->undo();
	}

	// Compare original
	Document *orig = open();
	QVERIFY(file->size() == doc->length());
	QVERIFY(compareDocument(doc, orig));
	delete orig;

	delete doc;
}

void TestDocument::testUndoReplace()
{
	Document *doc = open();
	QFile *file = doc->file();

	// check check length
	QVERIFY(file->size() == doc->length());

	// Replace data
	std::vector<uchar> dummy;
	const int SIZE = 1024 * 100;
	int insert_size = SIZE;
	for (int insert_pos = doc->length() / 2; insert_size > 0; ) {
		// assert: pos + len <= doc->lenght
		const int size = qMin(qMin(1024, qrand()), insert_size);
		// fix insert_pos
		while (insert_pos + size > doc->length()) {
			insert_pos -= qMin(insert_pos, qrand());
		}
		dummy.resize(size);
		doc->undoStack()->push(new ReplaceCommand(doc, insert_pos, size, &dummy[0], size));

		QVERIFY(doc->overwritable() == true);

		insert_pos = (insert_pos + size + qrand()) % doc->length();
		insert_size -= size;
	}

	
	// after replace
	QVERIFY(file->size() == doc->length());
	
	// Undo
	while (doc->undoStack()->canUndo()) {
		doc->undoStack()->undo();
	}
	QVERIFY(doc->overwritable() == true);

	// Compare original
	Document *orig = open();
	QVERIFY(file->size() == doc->length());
	QVERIFY(compareDocument(doc, orig));
	delete orig;

	delete doc;
}

void TestDocument::testUndoStack()
{
	// TODO: implement Document::reopenKeepUndo test case
}

void TestDocument::testDocumentSaveAs()
{
	for (int x = 0; x < WRITE_TEST_REPEAT; x++) {
		Document *doc = open();

		std::vector<uchar> dummy;
		const int SIZE = 1024 * 1024 * 2;
		int insert_size = SIZE;
		for (int insert_pos = doc->length() / 2; insert_size > 0; ) {
			const int size = qMin(qMax(1, (qrand() % 5000)), insert_size);
			dummy.resize(size);
			doc->undoStack()->push(new InsertCommand(doc, insert_pos, &dummy[0], size));
			insert_pos = (insert_pos + size + qrand()) % doc->length();
			insert_size -= size;
		}

		// check
		QVERIFY((doc->file()->size() + SIZE) == doc->length());

		// write file
		QTemporaryFile temp;
		QVERIFY2(temp.open(), "file open failed");
		temp.setAutoRemove(true);

		// write document
		QVERIFY(doc->write(&temp, NULL));
		QVERIFY(doc->overwritable() == false);	// returns false

		// check document
		QVERIFY(compareDocument(&temp, doc));

		delete doc;
	}
}


void TestDocument::testDocumentSave()
{
	for (int x = 0; x < WRITE_TEST_REPEAT; x++) {
		Document *doc = open();
		Document *doc_safe = open();

		std::vector<uchar> dummy;
		const int SIZE = 1024 * 1024 * 1;
		int insert_size = SIZE;
		for (int insert_pos = doc->length() / 2; insert_size > 0; ) {
			// assert: pos + len <= doc->lenght
			const int size = qMin(qMin(1024, qrand()), insert_size);
			// fix insert_pos
			while (insert_pos + size > doc->length()) {
				insert_pos -= qMin(insert_pos, qrand());
			}
			dummy.resize(size);
			doc->undoStack()->push(new ReplaceCommand(doc, insert_pos, size, &dummy[0], size));
			insert_pos = (insert_pos + size + qrand()) % doc->length();
			insert_size -= size;
		}

		QVERIFY(doc->overwritable());	// returns true

		// check
		QVERIFY(doc->file()->size() == doc->length());

		QVERIFY(doc->overwritable());	// returns true

		// write document
		QVERIFY(doc->write(NULL));
		// check document
		QVERIFY(compareDocument(doc->file(), doc_safe));

		// Undo
		while (doc->undoStack()->canUndo()) {
			doc->undoStack()->undo();
			doc_safe->undoStack()->undo();
		}
		// check document
		QVERIFY(compareDocument(doc->file(), doc));
		QVERIFY(compareDocument(doc, doc_safe));

		delete doc;
		delete doc_safe;
	}
}










QTEST_MAIN(TestDocument)
#include "document.moc"


