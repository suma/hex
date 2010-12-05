

#pragma once

#include <QMainWindow>
#include "ui_main.h"


class MainForm : public QMainWindow
{
	Q_OBJECT

public:
	MainForm();
	~MainForm();

protected:
	Ui::MainWindow ui;
};


