#ifndef STDHEXVIEW_H_INC
#define STDHEXVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../view.h"
#include "../highlight.h"
#include "cursor.h"
#include "caret.h"

namespace Standard {
	enum DrawMode {
		DRAW_ALL = 0,
		DRAW_LINE,
		DRAW_AFTER,
		DRAW_RANGE,	// [begin, end)
	};

	class HexConfig
	{
	private:
		uint Num;
	public:
		QRect Margin;
		QRect ByteMargin;
		QFont Font;
		QColor Colors[Color::ColorCount];

		bool EnableCaret;
		int CaretBlinkTime;

	private:
		QFontMetrics FontMetrics;
		std::vector<int> x_begin;	// pos of value
		std::vector<int> x_end;		// pos of end
		std::vector<int> x_area;
	
	public:
		HexConfig();

		inline uint getNum() const
		{
			return Num;
		}
		inline uint getNumV() const
		{
			return Num + 1;
		}
		inline void updateFont()
		{
			FontMetrics = QFontMetrics(Font);
		}
		inline int charWidth(int num = 1) const
		{
			return FontMetrics.width(QChar('A')) * num;
		}
		inline int charHeight() const
		{
			return FontMetrics.height();
		}
		inline int byteWidth() const
		{
			return ByteMargin.left() + charWidth(2) + ByteMargin.right();
		}
		inline int byteHeight() const
		{
			return ByteMargin.top() + FontMetrics.height() + ByteMargin.bottom();
		}
		inline const QRect &byteMargin() const
		{
			return ByteMargin;
		}
		inline const QFontMetrics &fontMetrics() const
		{
			return FontMetrics;
		}
		inline int top() const
		{
			return Margin.top();
		}
		inline int maxWidth() const
		{
			return X(Num - 1) + Margin.right();
		}
		inline int x(size_t i) const
		{
			Q_ASSERT(i < Num);
			return x_begin[i];
		}
		inline int X(size_t i) const
		{
			Q_ASSERT(i < Num);
			return x_end[i];
		}
		inline int caretWidth() const
		{
			return 3;
			//return ByteMargin.left() + charWidth();
		}
		inline int caretHeight() const
		{
			return byteHeight();
		}
		inline int width()
		{
			return byteWidth() * Num + Margin.left() + Margin.right();
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
				return conf.x(pos_) + conf.ByteMargin.left();
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
				return pos_ + conf.ByteMargin.top();
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

	class HexView : public ::View
	{
		Q_OBJECT

	public:
		HexView(QWidget *parent = NULL, Document *doc = NULL, Highlight *hi = NULL);
		~HexView();

		HexConfig & getConfig() { return config_; }
		void setCaretBlink(bool enable);


	private:
		struct CaretDrawInfo
		{
				QPainter &painter;
				CaretShape shape;
				QString hex;
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

		void movePosition(quint64 pos, bool sel, bool holdViewPos);
		void moveRelativePosition(qint64 pos, bool sel, bool holdViewPos);
		void redrawSelection(quint64 begin, quint64 end);

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

	public:
		void drawCaret(bool visible = true);
		void drawCaret(bool visible, quint64 pos);
	private:
		void drawCaretShape(CaretDrawInfo info);
		void drawCaretLine(const CaretDrawInfo &);
		void drawCaretFrame(const CaretDrawInfo &);
		void drawCaretBlock(const CaretDrawInfo &);
		void drawCaretUnderbar(const CaretDrawInfo &);

		void changeData(quint64 pos, uchar character, bool highNibble = false);
		void insertData(quint64 pos, uchar character);
		void removeData(quint64 pos, quint64 len);

	private:
		// Main components
		HexConfig config_;
		Cursor<HexView> *cursor_;
		Caret caret_;
	};

}

#endif
