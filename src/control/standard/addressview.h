
#pragma once

#include <QWidget>
#include "../color.h"
#include "local.h"

class Document;
class ScrollBar;

namespace Standard {

	class Global;
	class HexView;
	class TextView;
	class Cursor;
	class LayeredWidget;

	class AddressConfig : public QObject, public LocalConfig
	{
		Q_OBJECT

	private:
		QRect byteMargin_;

		// FIXME: separete 3 attributes 
		//  line
		//  column(depends hex, text)
		//  hex/text height

		bool column_visible_;
		bool line_visible_;

	public:
		AddressConfig(Global *global);
		~AddressConfig();

		QRect margin() const;
		QRect byteMargin() const;

		int drawableLines(int height) const;
		int top() const;

		int columnHeight() const;

		bool columnVisible() const;
		bool lineVisible() const;
		void setColumnVisible(bool);
		void setLineVisible(bool);

	signals:
		void fontChanged(QFont);
		void numChanged(uint);
		//void styleChanged();

	};

	class AddressView : public QWidget
	{
		Q_OBJECT

	public:
		AddressView(QWidget *parent, Global *global);
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

		void addHex(QWidget*);
		void addHexUnder(QWidget*);
		void addText(QWidget*);
		void addTextUnder(QWidget*);

		HexView *hexView() const;
		TextView *textView() const;

		//LayeredWidget *hexLayer() const;
		//LayeredWidget *textLayer() const;

	protected:

		void childEvent(QChildEvent *);
		void resizeEvent(QResizeEvent*);
		void focusInEvent(QFocusEvent*);

		bool eventFilter(QObject *obj, QEvent *event);

		// x
		int hexPos() const;
		int textPos() const;
		// y
		int y() const;

		void refreshScrollbarInfo();
	
	private slots:
		void topChanged(quint64);
		void positionChanged(quint64, quint64);

		// ScrollBar
		void valueChanged(qint64 value);

		// Document event
		void documentChanged();

	protected:
		AddressConfig config_;
		Global *global_;
		::ScrollBar *scrollbar_;
		::Document *document_;
		Cursor *cursor_;
		QWidget *last_focus_;
		HexView *hex_;
		TextView *text_;
		LayeredWidget *hex_layer_;
		LayeredWidget *text_layer_;

		bool column_visible_;
		bool line_visible_;
	
	};

}	// namespace Standard

