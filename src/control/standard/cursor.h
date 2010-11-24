
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
				position_ = pos;
				emit positionChanged(pos);
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
	
	public:

		template <class V>
		void movePosition(V view, quint64 pos, bool sel, bool holdViewPos)
		{
			// view depends on
			//   view->config()
			//   view->height()

			Q_ASSERT(pos <= document_->length());
			
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
					setTop(posLine - vwCountLine + 1);
				}
			} else {
				setTop(qMin(pos / view->config().getNum(), top_));
			}

			// Hold virtual position_ of caret
			if (holdViewPos) {
				const int vwNewPosLine = top_ - pos / view->config().getNum();
				const uint diff = qAbs(vwOldPosLine - vwNewPosLine);
				if (vwOldPosLine < vwNewPosLine) {
					if (diff < top_) {
						setTop(top_ - diff);
					} else {
						setTop(0);
					}
				} else {
					const quint64 maxTop = document_->length() / view->config().getNum() - vwCountLine + 1;
					if (top_ < std::numeric_limits<quint64>::max() - diff && top_ + diff <= maxTop) {
						setTop(top_ + diff);
					} else {
						setTop(maxTop);
					}
				}
			}

			setPosition(pos);
			setAnchor(sel ? anchor_ : position_);
		}


		quint64 getRelativePosition(qint64 pos) const
		{
			const quint64 diff = static_cast<quint64>(qAbs(pos));
			quint64 okPos = 0;
			if (pos < 0) {
				if (position_ < diff) {
					okPos = 0;
				} else {
					okPos = position_ - diff;
				}
			} else {
				if (position_ < std::numeric_limits<quint64>::max() - diff && position_ + diff <= document_->length()) {
					okPos = position_ + diff;
				} else {
					okPos = document_->length();
				}
			}
			
			return okPos;
		}

	signals:
		void topChanged(quint64);
		void positionChanged(quint64);
		void anchorChanged(quint64);
		void insertChanged(bool);

	};
}


