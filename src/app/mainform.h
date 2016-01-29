#pragma once

#include <QMainWindow>
#include "ui_main.h"


namespace Standard {
  class Editor;
}
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
  void saveAs(Standard::Editor *editor = NULL);

  void tabChanged(int index);
  void tabCloseRequested(int index);

protected:
  void closeEvent(QCloseEvent *event);

  Standard::Editor *currentEditor() const;
  Standard::Editor *editorAt(int index) const;

  bool openFile(QString);
  Document *saveFile(QString, Document *);
  void closeDocument(int index);

  int askAndDocumentSave(Document *document);

protected:
  Ui::MainWindow ui;
};

