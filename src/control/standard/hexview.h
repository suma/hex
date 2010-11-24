#ifndef STDHEXVIEW_H_INC
#define STDHEXVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "view.h"
#include "../highlight.h"
#include "cursor.h"
#include "caret.h"
#include "keyboard.h"

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
		uint num_;
		QRect margin_;
		QRect byteMargin_;
		QFont font_;
		int charWidth_;

		QColor colors_[Color::ColorCount];

	private:
		QFontMetrics fontMetrics_;
		std::vector<int> x_begin;	// pos of value
		std::vector<int> x_end;		// pos of end
		std::vector<int> x_area;

	public:
		bool EnableCaret;
		int CaretBlinkTime;
	
	public:
		HexConfig();

		uint getNum() const
		{
			return num_;
		}
		uint getNumV() const
		{
			return num_ + 1;
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
			return font_;
		}

		void updateFont()
		{
			fontMetrics_ = QFontMetrics(font_);
		}
		int charWidth(int num = 1) const
		{
			return charWidth_ * num;
		}
		int charHeight() const
		{
			return fontMetrics_.height();
		}
		int byteWidth() const
		{
			return byteMargin_.left() + charWidth(2) + byteMargin_.right();
		}
		int byteHeight() const
		{
			return byteMargin_.top() + fontMetrics_.height() + byteMargin_.bottom();
		}
		const QRect &margin() const
		{
			return margin_;
		}
		const QRect &byteMargin() const
		{
			return byteMargin_;
		}
		const QFontMetrics &fontMetrics() const
		{
			return fontMetrics_;
		}
		int top() const
		{
			return margin_.top();
		}
		int maxWidth() const
		{
			return X(num_ - 1) + margin_.right();
		}
		int x(size_t i) const
		{
			Q_ASSERT(i < num_);
			return x_begin[i];
		}
		int X(size_t i) const
		{
			Q_ASSERT(i < num_);
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
			return byteWidth() * num_ + margin_.left() + margin_.right();
		}
		int drawableLines(int height) const;
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
				pos_ = (pos_ + i) % conf.getNum();
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
		HexView(QWidget *parent = NULL, ::Document *doc = NULL, Highlight *hi = NULL);
		~HexView();

		HexConfig &config()
		{
			return config_;
		}

		Cursor &cursor() const
		{
			return *cursor_;
		}

		void setCaretBlink(bool enable);


	private:
		struct CaretDrawInfo
		{
				QPainter &painter;
				CaretShape shape;
				quint64 pos;
				bool caret_middle;
				int x;
				int y;

				CaretDrawInfo(QPainter &p, CaretShape shape, quint64 pos, int x, int y, bool caret_middle)
					: painter(p)
				{
					this->pos = pos;
					this->shape = shape;
					this->x = x;
					this->y = y;
					this->caret_middle = caret_middle;
				}
		};

	private:
		void resizeEvent(QResizeEvent *);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void timerEvent(QTimerEvent *);
		void keyPressEvent(QKeyEvent *);

		void redrawSelection(quint64 begin, quint64 end);

	public:
		void movePosition(quint64 pos, bool sel, bool holdViewPos);
		void moveRelativePosition(qint64 pos, bool sel, bool holdViewPos);

	public:
		// TODO change signal
		void drawView(DrawMode mode = DRAW_ALL, int = 0, int = 0);

	public:
		//void drawViewAfter(quint64 pos);

	signals:
		void viewDrawed(DrawMode mode, int, int);

	private:

		ColorType getColorType(const CursorSelection &, quint64 pos);
		void drawLines(QPainter &painter, quint64 top, int y, int x_begin, int x_end, uint size);

		void drawText(QPainter &painter, const QString &hex, int x, int y);

	private:

		static void byteToHex(uchar c, QString &h);
		quint64 posAt(const QPoint &pos) const;
		void caretDrawEvent(QPainter *painter);

	public:
		void drawCaret(bool visible = true);
		void drawCaret(bool visible, quint64 pos);
	private:
		void drawCaretShape(CaretDrawInfo info);
		void drawCaretLine(const CaretDrawInfo &);
		void drawCaretFrame(const CaretDrawInfo &);
		void drawCaretBlock(const CaretDrawInfo &);
		void drawCaretUnderbar(const CaretDrawInfo &);

	private slots:
		void inserted(quint64 pos, quint64 len);
		void removed(quint64 pos, quint64 len);

	private:
		// Main components
		HexConfig config_;
		Cursor *cursor_;
		Caret caret_;
		CaretDrawer *caret_drawer_;
		Keyboard *keyboard_;
		std::vector<uchar> buff_;
	};

}

#endif
