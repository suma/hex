

#include <QColor>
#include "global.h"
#include "cursor.h"
#include "editor.h"
#include "addressview.h"
#include "../util/util.h"

namespace Standard {

GlobalConfig::GlobalConfig()
	: num_(16)
	, margin_(2, 2, 3, 3)
	, byteMargin_(0, 0, 0, 0)
	, font_("Monaco", 17)
	, charWidth_(0)
	, fontMetrics_(font_)
{
	// Coloring
	colors_[Color::Background] = QColor(0xEF,0xEF,0xEF, 0);
	colors_[Color::Text] = QColor(0,0,0);
	colors_[Color::SelBackground] = QColor(0xA0,0xA0,0xFF, 170);
	colors_[Color::SelText] = QColor(0,0,0);
	colors_[Color::CaretBackground] = QColor(0xFF, 0, 0, 200);	// + transparency

	update();
}

void GlobalConfig::update()
{
	// get charWidth
	for (int i = 0; i < 16; i++) {
		QChar ch(util::itohex(i));
		charWidth_ = qMax(charWidth_, fontMetrics().boundingRect(ch).width());
	}
}

Global::Global(Editor *parent, ::Document *doc)
	: QObject(parent)
	, view_(parent)
	, document_(doc)
	, cursor_(new Cursor)
{
}

int Global::drawableLines() const
{
	return config_.drawableLines(view_->height());
}

}	// namesapce Standard
