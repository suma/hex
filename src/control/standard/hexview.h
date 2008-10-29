#ifndef STDHEXVIEW_H_INC
#define STDHEXVIEW_H_INC

#include <QFont>
#include <QFontMetrics>
#include "../view.h"

namespace Standard {

	class HexConfig
	{
	public:
		QRect Margin;
		QRect ByteMargin;
		QFont Font;
		QColor Colors[16];

	private:
		QFontMetrics FontMetrics;
		int x[16];
	
	public:
		HexConfig();

		void updateFont()
		{
			FontMetrics = QFontMetrics(Font);
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
