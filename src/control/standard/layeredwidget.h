#pragma once

#include <QWidget>
#include <QList>

namespace Standard {

  class LayeredWidget : public QWidget
  {
    Q_OBJECT

  protected:
    typedef QList<QWidget*> WidgetList;
    WidgetList layers_;


  public:
    LayeredWidget(QWidget *parent = NULL);
    virtual ~LayeredWidget();

    void add(QWidget *widget);
    void remove(QWidget *widget);

  protected:
    void childEvent(QChildEvent *);
    void resizeEvent(QResizeEvent*);
  };

}  // namespace
