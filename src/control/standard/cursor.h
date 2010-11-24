
#pragma once

#include <QtGlobal>
#include <QObject>
#include <algorithm>
#include <limits>
#include "../document.h"
#include "cursorutil.h"



namespace Standard {

	class Cursor : public QObject
	{
		Q_OBJECT

	private:
		::Document *document_;

		quint64 top_;		// Number of Line
		quint64 position_;	// pos(not line)
		quint64 anchor_;

		bool insert_;

		bool highNibble_;	// for hexview
	
	public:
		Cursor(::Document *doc)
			: document_(doc)
			, top_(0)
			, position_(0)
			, anchor_(0)
			, insert_(true)
			, highNibble_(true)
		{
		}

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
				qMin(position_, anchor_),	// begin
				qMax(position_, anchor_)	// end
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

		template <class V>
		void movePosition(V view, quint64 pos, bool sel, bool holdViewPos)
		{
			// view depends on
			//   view->config()
			//   view->height()

			Q_ASSERT(pos <= document_->length());
			quint64 top = top_;
			
			// Compute virtual position_ of caret
			int vwOldPosLine = 0;
			if (holdViewPos) {
				vwOldPosLine = top_ - position_ / view->config().getNum();
			}

			const uint vwCountLine = view->config().drawableLines(view->height()) - 1;

			//-- Update Cursor::top_ with position_
			const bool goDown = position_ < pos;
			if (goDown) {
				const quint64 posLine = pos / view->config().getNum();

				// if top_ + vwCountLine < posLine then Pos is invisible
				if (vwCountLine <= posLine && top_ <= posLine - vwCountLine) {
					top = (posLine - vwCountLine + 1);
				}
			} else {
				top = (qMin(pos / view->config().getNum(), top_));
			}

			// Hold virtual position_ of caret
			if (holdViewPos) {
				const int vwNewPosLine = top_ - pos / view->config().getNum();
				const uint diff = qAbs(vwOldPosLine - vwNewPosLine);
				if (vwOldPosLine < vwNewPosLine) {
					if (diff < top_) {
						top = top_ - diff;
					} else {
						top = 0;
					}
				} else {
					const quint64 maxTop = document_->length() / view->config().getNum() - vwCountLine + 1;
					if (top_ < std::numeric_limits<quint64>::max() - diff && top_ + diff <= maxTop) {
						top = top_ + diff;
					} else {
						top = maxTop;
					}
				}
			}

			if (top == top_) {
				//if ((!sel && hasSelection()) || sel && !hasSelection()) {
				if (!sel && hasSelection()) {
					// Redraw position only
					quint64 begin = qMin(qMin(position_, anchor_), pos);
					quint64 end = qMax(qMax(position_, anchor_), pos);
					setPosition(pos);
					setAnchor(sel ? anchor_ : position_);
					redrawSelection(begin, end);
				} else if (position_ != pos) {
					// Draw selection
					quint64 begin = qMin(qMin(position_, anchor_), pos);
					quint64 end   = qMax(qMax(position_, anchor_), pos);
					setPosition(pos);
					setAnchor(sel ? anchor_ : position_);
					redrawSelection(begin, end);
				}
			} else {
				// Redraw all
				setPosition(pos);
				setAnchor(sel ? anchor_ : position_);
				setTop(top);
			}
		}

		void connectTo(Cursor *cursor)
		{
			QObject::connect(this, SIGNAL(topChanged(quint64)), cursor,  SLOT(setTop(quint64)));
			QObject::connect(this, SIGNAL(positionChanged(quint64)), cursor,  SLOT(setPosition(quint64)));
			QObject::connect(this, SIGNAL(anchorChanged(quint64)), cursor,  SLOT(setAnchor(quint64)));
			QObject::connect(this, SIGNAL(insertChanged(bool)), cursor,  SLOT(setInsert(bool)));
			QObject::connect(this, SIGNAL(selectionUpdate(quint64, quint64, bool)), cursor,  SLOT(redrawSelection(quint64, quint64, bool)));
		}

		quint64 getRelativePosition(qint64 relative_pos) const
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
				if (position_ < std::numeric_limits<quint64>::max() - diff && position_ + diff <= document_->length()) {
					pos = position_ + diff;
				} else {
					pos = document_->length();
				}
			}
			
			return pos;
		}

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


