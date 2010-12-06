
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

	//connect(ui.actionNew_N, SIGNAL(activated()), this, SLOT(newDocument()));
}

MainForm::~MainForm()
{
}


void MainForm::newDocument()
{
}

void MainForm::open()
{
}

void MainForm::save()
{
}

void MainForm::saveAs()
{
}



