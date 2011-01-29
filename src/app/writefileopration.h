
#pragma once

#include <QString>
#include <QThread>
#include <QProgressDialog>
#include "control/document.h"

class QTimer;

class WriteFileOperation : public QThread, public Document::WriteCallback
{
	Q_OBJECT

private:
	QProgressDialog *progress_;
	QString path_;
	Document *document_;
	Document *result_;
	QTimer *timer_;
	quint64 write_size_;
	volatile quint64 completed_;
	bool reopen_document_;
	bool success_;

public:
	WriteFileOperation(QWidget *parent, QString path, Document *doc, bool reopen);
	~WriteFileOperation();

private:
	void run();
	void fail();
	void writeStarted(quint64 max);
	bool writeCallback(quint64 completed);
	void writeCompleted();

public:
	bool success() const;
	bool wasCanceled() const;
	Document *result() const;

private slots:
	void check();
	void cancel();
};

