

#pragma once

#include <QMainWindow>
#include "ui_main.h"


class Editor;
class Document;

class MainForm : public QMainWindow
{
	Q_OBJECT

public:
	MainForm();
	~MainForm();


protected slots:
	void newDocument();
	void open();
	void save();
	void saveAs(Editor *editor = NULL);

	void tabChanged(int index);
	void tabCloseRequested(int index);

protected:
	void closeEvent(QCloseEvent *event);

	Editor *currentEditor() const;
	Editor *editorAt(int index) const;

	bool openFile(QString);
	Document *saveFile(QString, Document *);
	void closeDocument(int index);

	int askDocumentSave(Document *document);

protected:
	Ui::MainWindow ui;
};


