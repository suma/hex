
#include <QtGui>
#include <QFileDialog>
#include <QFileInfo>
#include "editor.h"
#include "control/standard.h"
#include "control/document.h"
#include "mainform.h"
#include "editor.h"
#include "writefileopration.h"

MainForm::MainForm()
{
	ui.setupUi(this);

	// test
	Editor *editor = new Editor();
	ui.tabWidget->addTab(editor, QString("test"));
	ui.tabWidget->setDocumentMode(true);
	ui.tabWidget->setUsesScrollButtons(true);
	ui.tabWidget->setTabsClosable(true);
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
	connect(ui.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));

	editor->setFocus(Qt::ActiveWindowFocusReason);

	connect(ui.actionNew_N, SIGNAL(activated()), this, SLOT(newDocument()));
	connect(ui.actionOpen_O, SIGNAL(activated()), this, SLOT(open()));
	connect(ui.actionSave_As, SIGNAL(activated()), this, SLOT(saveAs()));
}

MainForm::~MainForm()
{
}


// slots
void MainForm::newDocument()
{
	MainForm *editor = new MainForm();
	editor->move(x() + 40, y() + 40);
	editor->show();
}

void MainForm::open()
{
	QFileDialog dialog(this);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setViewMode(QFileDialog::Detail);

	if (dialog.exec()) {
		QStringList files = dialog.selectedFiles();
		QStringList::Iterator it = files.begin();
		while (it != files.end()) {
			openFile(*it);
			++it;
		}

		// enable tab
		const int index = ui.tabWidget->count() - 1;
		if (index >= 0) {
			ui.tabWidget->setCurrentIndex(index);
		}
	}
}

void MainForm::save()
{
	Editor *editor = currentEditor();
	if (editor == NULL) {
		return;
	}
	Document *document = editor->document();
	Q_ASSERT(document != NULL);

	// save
}

void MainForm::saveAs()
{
	Editor *editor = currentEditor();
	if (editor == NULL) {
		return;
	}
	Document *document = editor->document();
	Q_ASSERT(document != NULL);

	QFileDialog dialog(this);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setViewMode(QFileDialog::Detail);

	if (dialog.exec()) {
		QStringList files = dialog.selectedFiles();
		QStringList::Iterator it = files.begin();
		if (it != files.end()) {
			Document *d = saveFile(*it, document);
			if (d != document) {
				//editor->setDocument(d);
			}
		}

		// TODO: change tab
	}
}


// internals

Editor *MainForm::currentEditor() const
{
	QWidget *widget = ui.tabWidget->currentWidget();
	if (widget == NULL) {
		return NULL;
	}

	return dynamic_cast<Editor*>(widget);
}

bool MainForm::openFile(QString path)
{
	QFile *file = new QFile(path);
	//QIODevice::ReadOnly QIODevice::ReadWrite
	if (!file->open(QIODevice::ReadWrite)) {
		// TODO: catch error message
		delete file;
		return false;
	}

	Document *document = new Document(file);
	Editor *editor = new Editor(this, document);

	ui.tabWidget->addTab(editor, QFileInfo(path).fileName());
}




Document *MainForm::saveFile(QString path, Document *document)
{
	Q_ASSERT(document != NULL);

	bool reopen_doc = false;
	WriteFileOperation operation(this, path, document, reopen_doc);
	
	if (!operation.wasCanceled() && !operation.success()) {
		// TODO: show error message
	}

	// TODO: return Tuple(success code, result doc);
	return operation.result();
}

void MainForm::tabChanged(int index)
{
	if (index == -1) {
		// Close QMainWindow when all tabs are closed
		close();
	}
}

void MainForm::tabCloseRequested(int index)
{
   qDebug() << "tabCloseRequest " << index;
   Q_ASSERT(index >= 0);

   // remove editor
   Editor *editor = currentEditor();
   Q_ASSERT(editor != NULL);

   // TODO: Check document modified using dialog
   delete editor;

   ui.tabWidget->removeTab(index);
}


