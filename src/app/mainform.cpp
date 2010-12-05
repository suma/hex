
#include <QtGui>
#include "editor.h"
#include "control/standard.h"
#include "control/document.h"
#include "mainform.h"
#include "editor.h"

MainForm::MainForm()
{
	ui.setupUi(this);

	Editor *editor = new Editor();
	ui.tabWidget->addTab(editor, QString("test"));
}

MainForm::~MainForm()
{
}


