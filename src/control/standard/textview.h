#ifndef STDTEXTVIEW_H_INC
#define STDTEXTVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../util/util.h"
#include "../view.h"
#include "../highlight.h"
#include "textcursor.h"
#include "caret.h"
#include "hexview.h"

namespace Standard {
	class TextDecodeHelper;

	class TextConfig
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
		TextConfig();

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
		inline int textWidth(const QString &string) const
		{
			return FontMetrics.width(string);
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
			return charWidth(1);
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
			return X(x_begin.size() - 1) + Margin.right();
		}
		inline int x(int i) const
		{
			Q_ASSERT(0 <= i && i < x_begin.size());
			return Margin.left() + x_begin[i];
		}
		inline int X(int i) const
		{
			Q_ASSERT(0 <= i && i < x_end.size());
			return Margin.left() + x_end[i];
		}
		inline int x_(int i) const
		{
			Q_ASSERT(0 <= i && i < x_begin.size());
			return x_begin[i];
		}
		inline int X_(int i) const
		{
			Q_ASSERT(0 <= i && i < x_end.size());
			return x_end[i];
		}
		inline int posWidth(int begin)
		{
			return x_end[begin] - x_begin[begin];
		}
		inline int posWidth(int begin, int end)
		{
			return x_end[end] - x_begin[begin];
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
			return charWidth(getNumV()) + Margin.left() + Margin.right();
		}
		int drawableLines(int height) const;
		int XToPos(int x) const;	// -1, 0..N => N + 2 patterns
		int YToLine(int y) const;	// -1, 0..N
		void update();

		class XIterator
		{
		private:
			const TextConfig &conf;
			int pos;

			// 次の行を描画するか表すフラグ
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
				return *this += 1;
			}

			XIterator &operator+=(uint i)
			{
				const uint old = pos;
				pos = (pos + i) % conf.getNum();
				set_next_flag(pos < old);
				return *this;
			}

			void operator++(int)
			{
				*this += 1;
			}

			int operator*() const
			{
				return pos;
			}

			int getTextX() const
			{
				return conf.x(pos);
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
		void drawText(QPainter &painter, const QString &str, int x, int y);

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
		Caret caret_;
		TextDecodeHelper *decode_helper_;
	};

}

#endif
