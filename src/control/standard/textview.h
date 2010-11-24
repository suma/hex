#ifndef STDTEXTVIEW_H_INC
#define STDTEXTVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../util/util.h"
#include "view.h"
#include "../highlight.h"
#include "caret.h"
#include "cursor.h"
#include "hexview.h"

namespace Standard {
	class TextDecodeHelper;
	class CaretDrawer;

	class TextConfig
	{
	private:
		uint num_;
		QRect margin_;
		QRect byteMargin_;
		QFont font_;
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
		TextConfig();

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
		int textWidth(const QString &string) const
		{
			return fontMetrics_.width(string);
		}
		int charWidth(int num = 1) const
		{
			return fontMetrics_.width(QChar('A')) * num;
		}
		int charHeight() const
		{
			return fontMetrics_.height();
		}
		int byteWidth() const
		{
			return charWidth(1);
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
			return X(x_begin.size() - 1) + margin_.right();
		}
		int x(size_t i) const
		{
			Q_ASSERT(i < x_begin.size());
			return margin_.left() + x_begin[i];
		}
		int X(size_t i) const
		{
			Q_ASSERT(i < x_end.size());
			return margin_.left() + x_end[i];
		}
		int x_(size_t i) const
		{
			Q_ASSERT(i < x_begin.size());
			return x_begin[i];
		}
		int X_(size_t i) const
		{
			Q_ASSERT(i < x_end.size());
			return x_end[i];
		}
		int posWidth(size_t begin)
		{
			return x_end[begin] - x_begin[begin];
		}
		int posWidth(size_t begin, size_t end)
		{
			return x_end[end] - x_begin[begin];
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
			return charWidth(getNumV()) + margin_.left() + margin_.right();
		}
		int drawableLines(int height) const;
		int XToPos(int x) const;	// -1, 0..N => N + 2 patterns
		int YToLine(int y) const;	// -1, 0..N
		void update();

		class XIterator
		{
		private:
			const TextConfig &conf;
			int pos_;

			// 次の行を描画するか表すフラグ
			bool next_flag_;
		public:
			XIterator(const TextConfig &conf, int pos)
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

			int textX() const
			{
				return conf.x(pos_);
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
			const TextConfig &conf;
			int pos_;
		public:
			YIterator(const TextConfig &conf, int pos)
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
				return pos_ + conf.byteMargin().top();
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

	class TextView : public View
	{
		Q_OBJECT

	public:
		TextView(QWidget *parent = NULL, ::Document *doc = NULL, Highlight *hi = NULL);
		~TextView();

		TextConfig &config()
		{
			return config_;
		}

		Caret &caret()
		{
			return caret_;
		}

		Cursor &cursor()
		{
			return *cursor_;
		}

		void setCaretBlink(bool enable);


	private:
		void resizeEvent(QResizeEvent *);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void timerEvent(QTimerEvent *);
		void keyPressEvent(QKeyEvent *);

		void inputMethodEvent(QInputMethodEvent *);
		QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

		void moveRelativePosition(qint64 pos, bool sel, bool holdViewPos);
		void redrawSelection(quint64 begin, quint64 end);

	public slots:
		// TODO change signal
		void drawView();

	private:

		void drawLines(QPainter &painter, quint64 top, int y, uint size);
		void drawText(QPainter &painter, const QString &hex, int x, int y, int charwidth);
		void drawText(QPainter &painter, const QString &str, int x, int y);

	private:

		quint64 posAt(const QPoint &pos) const;

		void caretDrawEvent(QPainter *painter);

	public:
		void drawCaret();

	private:
		void removeData(quint64 pos, quint64 len);

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
		TextConfig config_;
		Cursor *cursor_;
		TextDecodeHelper *decode_helper_;
		Caret caret_;
		CaretDrawer *caret_drawer_;
		std::vector<uchar> buff_;
	};

}

#endif
