
#pragma once

#include <QtGlobal>
#include <algorithm>
#include <limits>
#include "../document.h"
#include "cursorutil.h"



namespace Standard {

	class Cursor
	{
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

		void setTop(quint64 top)
		{
			top_ = top;
		}

		quint64 position() const
		{
			return position_;
		}

		void setPosition(quint64 pos)
		{
			position_ = pos;
		}

		quint64 anchor() const
		{
			return anchor_;
		}

		void setAnchor(quint64 anchor)
		{
			anchor_ = anchor;
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

		void setInsert(bool insert)
		{
			insert_ = insert;
		}

		void resetAnchor()
		{
			anchor_ = position_;
		}

		bool hasSelection()
		{
			return position_ != anchor_;
		}

		void reverseInsert()
		{
			insert_ = !insert_;
		}

		CursorSelection getSelection() const
		{
			CursorSelection c = {
				qMin(position_, anchor_),	// begin
				qMax(position_, anchor_)	// end
			};
			return c;
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
					top_ = posLine - vwCountLine + 1;
				}
			} else {
				top_ = qMin(pos / view->config().getNum(), top_);
			}

			// Hold virtual position_ of caret
			if (holdViewPos) {
				const int vwNewPosLine = top_ - pos / view->config().getNum();
				const uint diff = qAbs(vwOldPosLine - vwNewPosLine);
				if (vwOldPosLine < vwNewPosLine) {
					if (diff < top_) {
						top_ -= diff;
					} else {
						top_ = 0;
					}
				} else {
					const quint64 maxTop = document_->length() / view->config().getNum() - vwCountLine + 1;
					if (top_ < std::numeric_limits<quint64>::max() - diff && top_ + diff <= maxTop) {
						top_ += diff;
					} else {
						top_ = maxTop;
					}
				}
			}

			position_ = pos;
			anchor_ = sel ? anchor_ : position_;
		}


		quint64 getRelativePosition(qint64 pos)
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



	};
}


