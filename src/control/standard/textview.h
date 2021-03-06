#pragma once

#include <QFont>
#include <QFontMetrics>
#include "../util/util.h"
#include "view.h"
#include "global.h"
#include "local.h"
#include "caret.h"
#include "cursor.h"
#include "hexview.h"

class Document;

namespace Standard {
  class TextDecodeHelper;
  class CaretDrawer;
  class TextCaretDrawer;

  class TextConfig : public LocalConfig
  {
  private:
    QRect byteMargin_;

    std::vector<int> x_begin;  // pos of value
    std::vector<int> x_end;    // pos of end
    std::vector<int> x_area;

  private:
    uint numV() const
    {
      return global_->config().num() + 1;
    }

  public:
    TextConfig(Global *global);

    uint num() const
    {
      return global_->config().num();
    }

    int byteWidth() const
    {
      return charWidth(1);
    }
    const QRect &margin() const
    {
      return global_->config().margin();
    }
    const QRect &byteMargin() const
    {
      return byteMargin_;
    }
    int maxWidth() const
    {
      return X(x_begin.size() - 1) + margin().right();
    }
    int x(size_t i) const
    {
      Q_ASSERT(i < x_begin.size());
      return margin().left() + x_begin[i];
    }
    int X(size_t i) const
    {
      Q_ASSERT(i < x_end.size());
      return margin().left() + x_end[i];
    }
    int x_(size_t i) const
    {
      Q_ASSERT(i < x_begin.size());
      return x_begin[i];
    }
    int X_(size_t i) const
    {
      Q_ASSERT(i < x_end.size());
      return x_end[i];
    }
    int posWidth(size_t begin)
    {
      return x_end[begin] - x_begin[begin];
    }
    int posWidth(size_t begin, size_t end)
    {
      return x_end[end] - x_begin[begin];
    }
    int caretWidth() const
    {
      return 3;
      //return byteMargin_.left() + charWidth();
    }
    int caretHeight() const
    {
      return byteHeight();
    }
    int width()
    {
      return charWidth(numV()) + margin().left() + margin().right();
    }
    //int drawableLines(int height) const;
    int XToPos(int x) const;  // -1, 0..N => N + 2 patterns
    int YToLine(int y) const;  // -1, 0..N
    void update();

    class XIterator
    {
    private:
      const TextConfig &conf;
      int pos_;

      // 次の行を描画するか表すフラグ
      bool next_flag_;
    public:
      XIterator(const TextConfig &conf, int pos)
        : conf(conf)
        , pos_(pos)
        , next_flag_(false)
      {
      }

    public:
      XIterator operator++()
      {
        return *this += 1;
      }

      XIterator &operator+=(uint i)
      {
        const int old = pos_;
        pos_ = (pos_ + i) % conf.num();
        setNext(pos_ < old);
        return *this;
      }

      void operator++(int)
      {
        *this += 1;
      }

      int operator*() const
      {
        return pos_;
      }

      int textX() const
      {
        return conf.x(pos_);
      }

      bool isNext() const
      {
        return next_flag_;
      }

      void setNext(bool t)
      {
        next_flag_ = t;
      }
    };

    class YIterator
    {
    private:
      const TextConfig &conf;
      int pos_;
    public:
      YIterator(const TextConfig &conf, int pos)
        : conf(conf)
        , pos_(pos)
      {
      }

    public:
      YIterator operator++()
      {
        pos_ += conf.byteHeight();
        return *this;
      }

      void operator++(int)
      {
        pos_ += conf.byteHeight();
      }

      int operator*() const
      {
        return pos_;
      }

      int screenY() const
      {
        return pos_ + conf.byteMargin().top();
      }
    };

    XIterator createXIterator() const
    {
      return XIterator(*this, 0);
    }

    YIterator createYIterator(int pos) const
    {
      return YIterator(*this, pos);
    }
  };

  class TextView : public View
  {
    Q_OBJECT

  public:
    TextView(QWidget *parent, Global *global);
    ~TextView();

    TextConfig &config()
    {
      return config_;
    }

    Caret &caret()
    {
      return caret_;
    }

    Cursor &cursor()
    {
      return *cursor_;
    }

    CaretDrawer *createCaretWidget();

  private:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent *);

    void inputMethodEvent(QInputMethodEvent *);
    //QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    void moveRelativePosition(qint64 pos, bool sel, bool holdViewPos);
    void redrawSelection(quint64 begin, quint64 end);

  public slots:
    // TODO change signal
    void drawView();

  private:

    void drawLines(QPainter &painter, quint64 top, int y, uint size);
    void drawText(QPainter &painter, const QString &hex, int x, int y, int charwidth);
    void drawText(QPainter &painter, const QString &str, int x, int y);

  private:

    quint64 posAt(const QPoint &pos) const;

    void caretDrawEvent(QPainter *painter);

  private:
    void removeData(quint64 pos, quint64 len);

  private slots:
    void inserted(quint64 pos, quint64 len);
    void removed(quint64 pos, quint64 len);
    
    // cursor changed
    void topChanged(quint64);
    void positionChanged(quint64, quint64);
    void insertChanged(bool);
    void selectionUpdate(quint64, quint64);

  private:
    // Main components
    Global *global_;
    ::Document *document_;
    TextConfig config_;
    Cursor *cursor_;
    TextCaretDrawer *caret_drawer_;
    TextDecodeHelper *decode_helper_;
    Caret caret_;
    std::vector<uchar> buff_;
  };

}
