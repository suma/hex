#ifndef STDADDRESSVIEW_H_INC
#define STDADDRESSVIEW_H_INC

#include <QWidget>
#include "../color.h"

class Document;

namespace Standard {

	class HexView;
	class TextView;
	class Cursor;
	class LayeredWidget;

	class AddressConfig : public QObject
	{
		Q_OBJECT

	private:
		uint num_;
		QRect margin_;
		QRect byteMargin_;
		QFont font_;
		QFontMetrics fontMetrics_;
		QColor colors_[Color::ColorCount];

		// FIXME: separete 3 attributes 
		//  line
		//  column(depends hex, text)
		//  hex/text height

		bool column_visible_;
		bool line_visible_;

	public:
		AddressConfig();
		~AddressConfig();

		uint num() const;
		QRect margin() const;
		QRect byteMargin() const;
		QFont font() const;
		const QFontMetrics &fontMetrics() const;
		QColor color(size_t index) const;

		int charWidth(int num = 1) const
		{
			return fontMetrics_.width(QChar('A')) * num;
		}

		int drawableLines(int height) const;
		int top() const;
		int byteHeight() const;

		int columnHeight() const;

		bool columnVisible() const;
		bool lineVisible() const;
		void setColumnVisible(bool);
		void setLineVisible(bool);

	public slots:
		void setFont(QFont font);
		void setNum(int num);
		//void setStyle();	// FIXME

	signals:
		void fontChanged(QFont);
		void numChanged(uint);
		//void styleChanged();

	};

	class AddressView : public QWidget
	{
		Q_OBJECT

	public:
		AddressView(QWidget *parent, ::Document *doc);
		~AddressView();

		AddressConfig &config()
		{
			return config_;
		}

		void connect(Cursor *cursor);
		void paintEvent(QPaintEvent*);
		void drawColumn();
		void drawLine();

		void setHexView(HexView *hex);
		void setTextView(TextView *text);

		void addHexUnder(QWidget*);
		void addTextUnder(QWidget*);

		HexView *hexView() const;
		TextView *textView() const;

		//LayeredWidget *hexLayer() const;
		//LayeredWidget *textLayer() const;

	protected:

		void childEvent(QChildEvent *);
		void resizeEvent(QResizeEvent*);

		// x
		int hexPos() const;
		int textPos() const;
		// y
		int y() const;
	
	private slots:
		void topChanged(quint64);
		void positionChanged(quint64, quint64);

	protected:
		AddressConfig config_;
		::Document *document_;
		Cursor *cursor_;
		HexView *hex_;
		TextView *text_;
		LayeredWidget *hex_layer_;
		LayeredWidget *text_layer_;

		bool column_visible_;
		bool line_visible_;
	
	};

}


#endif
