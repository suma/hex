

#pragma once

#include <QMainWindow>
#include "ui_main.h"


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
	Ui::MainWindow ui;
};


