#pragma once

#include <QtGlobal>
#include <QObject>
#include <algorithm>
#include <limits>
#include "../document.h"
#include "cursorutil.h"
#include "global.h"


namespace Standard {

  class Cursor : public QObject
  {
    Q_OBJECT

    Q_PROPERTY(quint64 top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(quint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(quint64 anchor READ anchor WRITE setAnchor NOTIFY anchorChanged)
    Q_PROPERTY(bool insert READ insert WRITE setInsert NOTIFY insertChanged)
    Q_PROPERTY(bool nibble READ nibble WRITE setNibble)

  private:

    quint64 top_;    // Number of Line
    quint64 position_;  // pos(not line)
    quint64 anchor_;

    bool insert_;

    bool highNibble_;  // for hexview
  
  public:
    Cursor();

    quint64 top() const
    {
      return top_;
    }

    quint64 position() const
    {
      return position_;
    }

    quint64 anchor() const
    {
      return anchor_;
    }

    bool nibble() const
    {
      return highNibble_;
    }

    void setNibble(bool nibble)
    {
      highNibble_ = nibble;
    }

    void inverseNibble()
    {
      highNibble_ = !highNibble_;
    }

    bool insert() const
    {
      return insert_;
    }

    bool hasSelection()
    {
      return position_ != anchor_;
    }

    void reverseInsert()
    {
      setInsert(!insert_);
    }

    CursorSelection getSelection() const
    {
      CursorSelection c = {
        qMin(position_, anchor_),  // begin
        qMax(position_, anchor_),  // end
        anchor_
      };
      return c;
    }
    
    
  public slots:
    void setTop(quint64 top)
    {
      if (top_ != top) {
        top_ = top;
        emit topChanged(top);
      }
    }

    void setPosition(quint64 pos)
    {
      if (position_ != pos) {
        quint64 old = position_;
        position_ = pos;
        emit positionChanged(pos);
        emit positionChanged(old, pos);
      }
    }

    void setAnchor(quint64 anchor)
    {
      if (anchor_ != anchor) {
        anchor_ = anchor;
        emit anchorChanged(anchor);
      }
    }

    void setInsert(bool insert)
    {
      if (insert_ != insert) {
        insert_ = insert;
        emit insertChanged(insert);
      }
    }

    void resetAnchor()
    {
      if (anchor_ != position_) {
        anchor_ = position_;
        emit anchorChanged(position_);
      }
    }

    void redrawSelection(quint64 begin, quint64 end, bool fire = true)
    {
      emit selectionUpdate(begin, end);
      if (fire) {
        emit selectionUpdate(begin, end, false);
      }
    }

  public:
    void movePosition(Global *global, int height, quint64 pos, bool sel, bool holdViewPos);
    void connectTo(Cursor *cursor);
    quint64 getRelativePosition(qint64 relative_pos, ::Document *document_) const;

  signals:
    void topChanged(quint64);
    void positionChanged(quint64);
    void positionChanged(quint64 old, quint64 pos);
    void anchorChanged(quint64);
    void insertChanged(bool);

    // for view
    void selectionUpdate(quint64, quint64);
    void selectionUpdate(quint64, quint64, bool);

  };
}
