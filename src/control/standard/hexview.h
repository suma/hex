
#pragma once

#include <QFont>
#include <QFontMetrics>
#include "view.h"
#include "global.h"
#include "cursor.h"
#include "caret.h"
#include "keyboard.h"

class Document;

namespace Standard {

	class CaretDrawer;

	enum DrawMode {
		DRAW_ALL = 0,
		DRAW_LINE,
		DRAW_AFTER,
		DRAW_RANGE,	// [begin, end)
	};

	class HexConfig
	{
	private:
		Global *global_;
		QRect byteMargin_;

		QColor colors_[Color::ColorCount];

	private:
		std::vector<int> x_begin;	// pos of value
		std::vector<int> x_end;		// pos of end
		std::vector<int> x_area;
	
		uint getNumV() const
		{
			return global_->config().num() + 1;
		}

	public:
		HexConfig(Global *global);

		uint num() const
		{
			return global_->config().num();
		}
		QColor color(size_t index) const
		{
			Q_ASSERT(index < Color::ColorCount);
			return colors_[index];
		}

		void setColor(size_t index, QColor color)
		{
			Q_ASSERT(index < Color::ColorCount);
			colors_[index] = color;
		}

		const QFont &font() const
		{
			return global_->config().font();
		}

		void updateFont()
		{
			global_->config().updateFont();
		}
		int charWidth(int num = 1) const
		{
			return global_->config().charWidth(num);
		}
		int charHeight() const
		{
			return global_->config().charHeight();
		}
		int byteWidth() const
		{
			return byteMargin_.left() + charWidth(2) + byteMargin_.right();
		}
		int byteHeight() const
		{
			return global_->config().byteHeight();
		}
		const QRect &margin() const
		{
			return global_->config().margin();
		}
		const QRect &byteMargin() const
		{
			return byteMargin_;
		}
		const QFontMetrics &fontMetrics() const
		{
			return global_->config().fontMetrics();
		}
		int top() const
		{
			return global_->config().top();
		}
		int maxWidth() const
		{
			return X(num() - 1) + margin().right();
		}
		int x(size_t i) const
		{
			Q_ASSERT(i < num());
			return x_begin[i];
		}
		int X(size_t i) const
		{
			Q_ASSERT(i < num());
			return x_end[i];
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
			return byteWidth() * num() + margin().left() + margin().right();
		}
		//int drawableLines(int height) const;
		int XToPos(int x) const;	// -1, 0..N => N + 1 patterns
		int YToLine(int y) const;	// -1, 0..N
		void update();

		class XIterator
		{
		private:
			const HexConfig &conf;
			int pos_;
			bool next_flag_;
		public:
			XIterator(const HexConfig &conf, int pos)
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

			int screenX() const
			{
				return conf.x(pos_);
			}

			int textX() const
			{
				return conf.x(pos_) + conf.byteMargin().left();
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
			const HexConfig &conf;
			int pos_;
		public:
			YIterator(const HexConfig &conf, int pos)
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
				return pos_ + conf.byteMargin_.top();
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

	class HexView : public View
	{
		Q_OBJECT

	public:
		HexView(QWidget *parent, Global *global);
		~HexView();

		HexConfig &config()
		{
			return config_;
		}

		Caret &caret()
		{
			return caret_;
		}

		Cursor &cursor() const
		{
			return *cursor_;
		}

		CaretDrawer * createCaretWidget();

	private:
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void keyPressEvent(QKeyEvent *);

		void redrawSelection(quint64 begin, quint64 end);

	public:
		//void movePosition(quint64 pos, bool sel, bool holdViewPos);
		void moveRelativePosition(qint64 pos, bool sel, bool holdViewPos);

	public:
		// TODO change signal
		void drawView(DrawMode mode = DRAW_ALL, int = 0, int = 0);

	public:
		//void drawViewAfter(quint64 pos);

	private:

		void drawLines(QPainter &painter, quint64 top, int y, int x_begin, int x_end, uint size);

		void drawText(QPainter &painter, const QString &hex, int x, int y);

	private:

		static void byteToHex(uchar c, QString &h);
		quint64 posAt(const QPoint &pos) const;


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
		HexConfig config_;
		Cursor *cursor_;
		Caret caret_;
		Keyboard *keyboard_;
		std::vector<uchar> buff_;
	};

}


