
#pragma once

namespace Color {
	enum color {
		Background = 0,
		Text,
		SelBackground,	// hilighted
		SelText,
		CaretBackground,
		ColorCount,
	};
}


struct ColorType {
	int Background;
	int Text;
	ColorType(int bg_color = Color::Background, int text_color = Color::Text)
	{
		Background = bg_color;
		Text = text_color;
	}
};


