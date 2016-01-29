#include "cursor.h"
#include "../document.h"

namespace Standard {

Cursor::Cursor()
  : top_(0)
  , position_(0)
  , anchor_(0)
  , insert_(true)
  , highNibble_(true)
{
}


void Cursor::movePosition(Global *global, int height, quint64 pos, bool sel, bool holdViewPos)
{
  ::Document *doc = global->document();
  // view depends on
  //   view->config()
  //   view->height()

  Q_ASSERT(pos <= doc->length());
  quint64 top = top_;
  const int NUM = global->config().num();
  
  // Compute virtual position_ of caret
  int vwOldPosLine = 0;
  if (holdViewPos) {
    vwOldPosLine = top_ - position_ / NUM;
  }

  const uint vwCountLine = global->config().drawableLines(height) - 1;

  //-- Update Cursor::top_ with position_
  const bool goDown = position_ < pos;
  if (goDown) {
    const quint64 posLine = pos / NUM;

    // if top_ + vwCountLine < posLine then Pos is invisible
    if (vwCountLine <= posLine && top_ <= posLine - vwCountLine) {
      top = (posLine - vwCountLine + 1);
    }
  } else {
    top = (qMin(pos / NUM, top_));
  }

  // Hold virtual position_ of caret
  if (holdViewPos) {
    const int vwNewPosLine = top_ - pos / NUM;
    const uint diff = qAbs(vwOldPosLine - vwNewPosLine);
    if (vwOldPosLine < vwNewPosLine) {
      if (diff < top_) {
        top = top_ - diff;
      } else {
        top = 0;
      }
    } else {
      const quint64 maxTop = doc->length() / NUM - vwCountLine + 1;
      if (top_ < std::numeric_limits<quint64>::max() - diff && top_ + diff <= maxTop) {
        top = top_ + diff;
      } else {
        top = maxTop;
      }
    }
  }

  if (top == top_) {
    if (!sel && sel == hasSelection()) {
      setPosition(pos);
      setAnchor(pos);
    } else {
      // Redraw position only
      quint64 begin = qMin(qMin(position_, anchor_), pos);
      quint64 end = qMax(qMax(position_, anchor_), pos);
      if (sel != hasSelection()) {
        setAnchor(sel ? anchor_ : pos);
      }
      setPosition(pos);
      redrawSelection(begin, end);
    }
  } else {
    // Redraw all
    setPosition(pos);
    setAnchor(sel ? anchor_ : position_);
    setTop(top);
  }
}

void Cursor::connectTo(Cursor *cursor)
{
  QObject::connect(this, SIGNAL(topChanged(quint64)), cursor,  SLOT(setTop(quint64)));
  QObject::connect(this, SIGNAL(positionChanged(quint64)), cursor,  SLOT(setPosition(quint64)));
  QObject::connect(this, SIGNAL(anchorChanged(quint64)), cursor,  SLOT(setAnchor(quint64)));
  QObject::connect(this, SIGNAL(insertChanged(bool)), cursor,  SLOT(setInsert(bool)));
  QObject::connect(this, SIGNAL(selectionUpdate(quint64, quint64, bool)), cursor,  SLOT(redrawSelection(quint64, quint64, bool)));
}

quint64 Cursor::getRelativePosition(qint64 relative_pos, ::Document *doc) const
{
  const quint64 diff = static_cast<quint64>(qAbs(relative_pos));
  quint64 pos = 0;
  if (relative_pos < 0) {
    if (position_ < diff) {
      pos = 0;
    } else {
      pos = position_ - diff;
    }
  } else {
    if (position_ < std::numeric_limits<quint64>::max() - diff && position_ + diff <= doc->length()) {
      pos = position_ + diff;
    } else {
      pos = doc->length();
    }
  }
  
  return pos;
}


}  // nemaspace Standard
