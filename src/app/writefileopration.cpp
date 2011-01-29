
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include "writefileopration.h"


WriteFileOperation::WriteFileOperation(QWidget *parent, QString path, Document *doc, bool reopen)
	: QThread(parent)
	, path_(path)
	, document_(doc)
	, result_(doc)
	, timer_(NULL)
	, write_size_(0)
	, completed_(0)
	, reopen_document_(reopen)
	, success_(false)
{
	progress_ = new QProgressDialog();
	progress_->setRange(0, 100);
	progress_->setLabelText("Writing file");
	progress_->setCancelButtonText("Cancel");
	progress_->setAutoClose(true);
	connect(progress_, SIGNAL(canceled()), this, SLOT(cancel()));

	timer_ = new QTimer(this);
	connect(timer_, SIGNAL(timeout()), this, SLOT(check()));
	timer_->start(300);

	QThread::start();
	progress_->exec();
}

WriteFileOperation::~WriteFileOperation()
{
}

void WriteFileOperation::run()
{
	// open file
	QFile *file = new QFile(path_);
	if (!file->open(QIODevice::ReadWrite)) {
		delete file;
		return;
	}

	// start writing file
	document_->write(file, this);

	if (reopen_document_) {
		// reopen Document
		size_t max_buffer = 1024 * 1024;	// FIXME: transfered Undo/Redo buffer size
		result_ = Document::reopenKeepUndo(document_, file, max_buffer);
	} else {
		delete file;
	}

	// write successful
	success_ = true;
}

void WriteFileOperation::fail()
{
	// finished
	completed_ = write_size_;
}

void WriteFileOperation::writeStarted(quint64 max)
{
	write_size_ = max;
	progress_->setValue(0);
}

bool WriteFileOperation::writeCallback(quint64 completed)
{
	completed_ = completed;
	return wasCanceled();
}

void WriteFileOperation::writeCompleted()
{
	completed_ = write_size_;
}

bool WriteFileOperation::wasCanceled() const
{
	return progress_->wasCanceled();
}

bool WriteFileOperation::success() const
{
	return success_;
}

Document *WriteFileOperation::result() const
{
	return result_;
}

void WriteFileOperation::check()
{
	if (completed_ >= write_size_) {
		// completed
		progress_->setValue(100);
		timer_->stop();
	} else {
		// update progress
		const int v = static_cast<int>(((float)completed_ / (float)write_size_) * 100);

		progress_->setValue(v);
	}
}

void WriteFileOperation::cancel()
{
	// stop timer when canceled
	timer_->stop();
}

