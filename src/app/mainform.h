

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
	void saveAs();

protected:

	Editor *currentEditor() const;

	bool openFile(QString);
	void saveFile(QString, Document *);

protected:
	Ui::MainWindow ui;
};


