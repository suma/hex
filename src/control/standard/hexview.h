#ifndef STDHEXVIEW_H_INC
#define STDHEXVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../view.h"
#include "../highlight.h"


namespace Standard {

	class Cursor;
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
		QColor HexCaretColor;
		int Spaces[Num+1]; // last is yoyuu

		bool EnableCaret;
		int CaretBlinkTime;

	private:
		QFontMetrics FontMetrics;
		int top_;
		int x_[Num];	// pos of value
		int X_[Num];	// pos of end
		int xarea_[Num+1];
	
	public:
		HexConfig();

		inline void updateFont()
		{
			FontMetrics = QFontMetrics(Font);
		}
		inline int charWidth(int num = 1) const
		{
			return FontMetrics.maxWidth() * num;
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
			return top_;
		}
		inline int maxWidth() const
		{
			return X(Num - 1) + Margin.right();
		}
		inline int x(int i) const
		{
			Q_ASSERT(0 <= i && i < Num);
			return x_[i];
		}
		inline int X(int i) const
		{
			Q_ASSERT(0 <= i && i < Num);
			return X_[i];
		}
		inline int caretWidth() const
		{
			return charWidth();
		}
		inline int caretHeight() const
		{
			return byteHeight();
		}
		int drawableLines(int height) const;
		int drawableLinesN(int height) const;
		int XToPos(int x) const;	// -1, 0..15, 16 => 18 patterns
		int YToLine(int y) const;	// -1, 0..N
		void calculate();
	};

	class HexView : public ::View
	{
		Q_OBJECT

	public:
		HexView(QWidget *parent = NULL, Document *doc = NULL, Cursor *cur = NULL, Highlight *hi = NULL);

		HexConfig & getConfig() { return config_; }
		void setCaretBlink(bool enable);

	protected:

		void refreshPixmap();
		void refreshPixmap(int type, int = 0, int = 0);
		void drawLines(QPainter &painter, int y, int ytop);

		void resizeEvent(QResizeEvent *);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void timerEvent(QTimerEvent *);

		void isSelected(bool &selected, quint64 &sb, quint64 &se, quint64 top, int yCount, uint size);
		void drawSelected(bool reset = false);
		void drawCaret(bool visible, quint64 pos, int ytop, int ymax);
		void redrawCaret();

		void byteToHex(uchar c, QString &h);
		quint64 moveByMouse(int x, int y);

	public slots:
		void drawCaret(bool visible);
		void drawCaret(bool visible, quint64 pos);

	signals:
		//connect to drawCaret(visible, pos);
		void caretChanged(bool visible, quint64 pos);

	protected:
		// Main components
		HexConfig config_;
		Cursor *cur_;
		QPixmap off_;
	};

}

#endif
