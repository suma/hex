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
		int Spaces[Num+1]; // last is yoyuu

	private:
		QFontMetrics FontMetrics;
		int top_;
		int x_[Num];	// pos of value
		int X_[Num];	// pos of end
		int xarea_[Num];
	
	public:
		HexConfig();

		inline void updateFont()
		{
			FontMetrics = QFontMetrics(Font);
		}
		inline int byteWidth() const
		{
			return ByteMargin.left() + (FontMetrics.maxWidth() * 2) + ByteMargin.right();
		}
		inline int byteHeight() const
		{
			return ByteMargin.top() + FontMetrics.height() + ByteMargin.bottom();
		}
		inline const QRect &byteMargin() const
		{
			return ByteMargin;
		}
		inline int byteEnd(int i) const
		{
			return x(i) + (FontMetrics.maxWidth() * 2) + ByteMargin.right();
		}
		inline const QFontMetrics &fontMetrics() const
		{
			return FontMetrics;
		}
		inline int top() const
		{
			return top_;
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
		int toPos(int x);	// -1, 0..31, 32:: 32 + 2 = 34
		int toLine(int y);	// [0, N]
		void calculate();
	};

	class HexView : public ::View
	{
		Q_OBJECT

	public:
		HexView(QWidget *parent = NULL, Document *doc = NULL, Cursor *cur = NULL, Highlight *hi = NULL);

		HexConfig & getConfig() { return config_; }

	protected:

		void refreshPixmap();
		void refreshPixmap(int);

		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);

		void byteToHex(uchar c, QString &h);

	protected:
		// Main components
		HexConfig config_;
		Cursor *cur_;
	};

}

#endif
