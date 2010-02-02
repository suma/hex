#ifndef STDHEXVIEW_H_INC
#define STDHEXVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../view.h"
#include "../highlight.h"
#include "hexcursor.h"

namespace Standard {
	enum DrawMode {
		DRAW_ALL = 0,
		DRAW_LINE,
		DRAW_AFTER,
		DRAW_RANGE,	// [begin, end)
	};

	class HexConfig
	{
	public:
		enum {
			Num = 16,
		};
		QRect Margin;
		QRect ByteMargin;
		QFont Font;
		QColor Colors[Color::ColorCount];

		bool EnableCaret;
		int CaretBlinkTime;

	private:
		QFontMetrics FontMetrics;
		int x_begin[Num];	// pos of value
		int x_end[Num];		// pos of end
		int x_area[Num];
	
	public:
		HexConfig();

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
		inline int x(int i) const
		{
			Q_ASSERT(0 <= i && i < Num);
			return x_begin[i];
		}
		inline int X(int i) const
		{
			Q_ASSERT(0 <= i && i < Num);
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
		int XToPos(int x) const;	// -1, 0..15, 16 => 18 patterns
		int YToLine(int y) const;	// -1, 0..N
		void update();

		class XIterator
		{
		private:
			const HexConfig &conf;
			int pos;
		public:
			XIterator(const HexConfig &conf, int pos)
				: conf(conf)
				, pos(pos)
			{
			}

		public:
			XIterator operator++()
			{
				pos = (pos + 1) % HexConfig::Num;
				return *this;
			}

			void operator++(int)
			{
				pos = (pos + 1) % HexConfig::Num;
			}

			int operator*() const
			{
				return pos;
			}

			int getScreenX() const
			{
				return conf.x(pos);
			}

			int getTextX() const
			{
				return conf.x(pos) + conf.ByteMargin.left();
			}
		};

		class YIterator
		{
		private:
			const HexConfig &conf;
			int pos;
		public:
			YIterator(const HexConfig &conf, int pos)
				: conf(conf)
				, pos(pos)
			{
			}

		public:
			YIterator operator++()
			{
				pos += conf.byteHeight();
				return *this;
			}

			void operator++(int)
			{
				pos += conf.byteHeight();
			}

			int operator*() const
			{
				return pos;
			}

			int getScreenY() const
			{
				return pos + conf.ByteMargin.top();
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
		HexCursor & getCursor() { return *cursor_; }
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

	public:
		// TODO change signal
		void drawView(DrawMode mode = DRAW_ALL, int = 0, int = 0);

	public:
		//void drawViewAfter(quint64 pos);

	signals:
		void viewDrawed(DrawMode mode, int, int);

	private:
		void drawLines(QPainter &painter, DCIList &dcolors, int y, int x_begin = 0, int x_end = HexConfig::Num);	// x: [)
		void drawText(QPainter &painter, const QString &hex, int x, int y);


		void isSelected(bool &selected, quint64 &sb, quint64 &se, quint64 top, int yCount, uint size);
		bool isSelected(quint64 pos) const;
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
		HexCursor *cursor_;
	};

}

#endif
