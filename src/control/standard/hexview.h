#ifndef STDHEXVIEW_H_INC
#define STDHEXVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../view.h"

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
		QColor Colors[16];
		int Spaces[Num+1]; // last is yoyuu

	private:
		QFontMetrics FontMetrics;
		int top_;
		int x_[Num];	// pos of value
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
		inline int top() const
		{
			return top_;
		}
		inline int x(int i) const
		{
			Q_ASSERT(0 <= i && i < Num);
			return x_[i];
		}
		int toPos(int x);	// -1, 0..31, 32:: 32 + 2 = 34
		int toLine(int y);	// [0, N]
		void calculate();
	};

	class HexView : public ::View
	{
		Q_OBJECT

	public:
		HexView(QWidget *parent = NULL, Document *doc = NULL, Cursor *cur = NULL);

		HexConfig & getConfig() { return config_; }

	protected:

		void refreshPixmap();
		void refreshPixmap(int);

		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);

		struct DrawInfo {
			int y;
			int count;
			int xb;
			int xe;
			uint size;
			quint64 top;
			quint64 sb;
			quint64 se;
			std::vector<uchar> buff;
			DrawInfo(int Y, quint64 Top, int Count, int Xb, int Xe, quint64 Sb, quint64 Se, uint Size)
				: buff(size)
			{
				y = Y;
				count = Count;
				xb = Xb;
				Xe = Xe;
				size = Size;
				top = Top;
				sb = Sb;
				se = Se;
			}
			DrawInfo() {}
			~DrawInfo() {}
		};


		void drawSelected(const DrawInfo &di);
		void drawNoSelected(const DrawInfo &di);

	protected:
		HexConfig config_;
		Cursor *cur_;
	};

}

#endif
