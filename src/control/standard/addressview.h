#ifndef STDADDRESSVIEW_H_INC
#define STDADDRESSVIEW_H_INC

#include <QWidget>
#include "../color.h"

class Document;

namespace Standard {

	class HexView;
	class HexConfig;
	class Cursor;

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

	public:
		AddressConfig();
		~AddressConfig();

		uint num() const;
		QRect margin() const;
		QRect byteMargin() const;
		QFont font() const;
		const QFontMetrics &fontMetrics() const;
		QColor color(size_t index) const;

		int drawableLines(int height) const;
		int top() const;
		int byteHeight() const;

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
		AddressView(QWidget *parent, ::Document *doc, HexView *view);
		~AddressView();


		void paintEvent(QPaintEvent*);
		void drawView();

	private:

	
	private slots:
		void topChanged(quint64);
		void positionChanged(quint64, quint64);

	protected:
		AddressConfig config_;
		::Document *document_;
		HexView *view_;
		const Cursor &cursor_;

		bool line_visible_;
	
	};

}


#endif
