#pragma once

#include <QObject>
#include <QFont>
#include <QFontMetrics>
#include "../color.h"
#include "../document.h"

namespace Standard {

	class Cursor;
	class Editor;

	class GlobalConfig : public QObject
	{
		Q_OBJECT

	public:
		GlobalConfig();
		void update();

		uint num() const
		{
			return num_;
		}

		uint numV() const
		{
			return num_ + 1;
		}

		const QFont &font() const
		{
			return font_;
		}

		void updateFont()
		{
			fontMetrics_ = QFontMetrics(font_);
			update();
		}

		int charWidth(int num = 1) const
		{
			return charWidth_ * num;
		}
		int charHeight() const
		{
			return fontMetrics_.height();
		}

		const QFontMetrics &fontMetrics() const
		{
			return fontMetrics_;
		}

		const QRect &margin() const
		{
			return margin_;
		}

		const QRect &byteMargin() const
		{
			return byteMargin_;
		}

		int top() const
		{
			return margin_.top();
		}

		int byteHeight() const
		{
			return byteMargin_.top() + fontMetrics_.height() + byteMargin_.bottom();
		}

		int drawableLines(int height) const
		{
			const int y = top() + byteMargin_.top();
			return (height - y + byteHeight()) / byteHeight();
		}
	private:
		uint num_;
		QRect margin_;
		QRect byteMargin_;
		QFont font_;
		int charWidth_;

		QColor colors_[Color::ColorCount];
		QFontMetrics fontMetrics_;
	};

	class Global : public QObject
	{
		Q_OBJECT
	public:
		Global(Editor *parent, ::Document *document);


		const GlobalConfig &config() const
		{
			return config_;
		}

		GlobalConfig &config()
		{
			return config_;
		}

		Cursor &cursor()
		{
			return *cursor_;
		}

		::Document *document() const
		{
			return document_;
		}

		int drawableLines() const;

	private:
		Editor *view_;
		::Document *document_;
		GlobalConfig config_;
		Cursor *cursor_;
	};




}	// namespace Standard
