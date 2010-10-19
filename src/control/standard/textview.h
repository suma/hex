#ifndef STDTEXTVIEW_H_INC
#define STDTEXTVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../view.h"
#include "../highlight.h"
#include "textcursor.h"
#include "hexview.h"

namespace Standard {
	class TextDecodeHelper;

	class TextConfig
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
		TextConfig();

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
			return ByteMargin.left() + charWidth(1) + ByteMargin.right();
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
		int drawableLines(int height) const;
		int XToPos(int x) const;	// -1, 0..15, 16 => 18 patterns
		int YToLine(int y) const;	// -1, 0..N
		void update();

		class XIterator
		{
		private:
			const TextConfig &conf;
			int pos;
			bool next_flag_;
		public:
			XIterator(const TextConfig &conf, int pos)
				: conf(conf)
				, pos(pos)
				, next_flag_(false)
			{
			}

		public:
			XIterator operator++()
			{
				//pos = (pos + 1) % TextConfig::Num;
				//return *this;
				return *this += 1;
			}

			XIterator &operator+=(uint i)
			{
				const uint old = pos;
				pos = (pos + i) % TextConfig::Num;
				set_next_flag(pos < old);
				return *this;
			}

			void operator++(int)
			{
				//pos = (pos + 1) % TextConfig::Num;
				*this += 1;
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

			bool is_next_flag() const
			{
				return next_flag_;
			}

			void set_next_flag(bool t)
			{
				next_flag_ = t;
			}
		};

		class YIterator
		{
		private:
			const TextConfig &conf;
			int pos;
		public:
			YIterator(const TextConfig &conf, int pos)
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

	class TextView : public ::View
	{
		Q_OBJECT

	public:
		TextView(QWidget *parent = NULL, Document *doc = NULL, Highlight *hi = NULL);
		~TextView();

		TextConfig & getConfig() { return config_; }
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


	public slots:
		// TODO change signal
		void drawView();

	private:

		ColorType getColorType(const CursorSelection &, quint64 pos);
		void drawLines(QPainter &painter, quint64 top, int y, uint size);
		void drawText(QPainter &painter, const QString &hex, int x, int y, int charwidth);

	private:

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

		void changeData(quint64 pos, uchar character);
		void insertData(quint64 pos, uchar character);
		void removeData(quint64 pos, quint64 len);

	private:
		// Main components
		TextConfig config_;
		TextCursor *cursor_;
		TextDecodeHelper *decode_helper_;
	};

}

#endif
