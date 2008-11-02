#ifndef STDHEXVIEW_H_INC
#define STDHEXVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../view.h"

namespace Standard {

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
		int Spaces[Num];

	private:
		QFontMetrics FontMetrics;
		int top_;
		int x_[Num];
	
	public:
		HexConfig();

		inline void updateFont()
		{
			FontMetrics = QFontMetrics(Font);
		}
		inline int byteWidth() const
		{
			return ByteMargin.left() + FontMetrics.maxWidth() + ByteMargin.right();
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
		void calculate();
	};

	class HexView : public ::View
	{
		Q_OBJECT

	public:
		HexView(QWidget *parent = NULL, Document *doc = NULL);

		HexConfig & getConfig() { return config_; }

	protected:

		void refreshPixmap();

	protected:
		HexConfig config_;
	};

}

#endif
