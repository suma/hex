
#include "local.h"

namespace Standard {

LocalConfig::LocalConfig(Global *global)
	: global_(global)
{
}

LocalConfig::~LocalConfig()
{
}

const QFont &LocalConfig::font() const
{
	return global_->config().font();
}

uint LocalConfig::num() const
{
	return global_->config().num();
}

void LocalConfig::updateFont()
{
	global_->config().updateFont();
}

int LocalConfig::textWidth(const QString &string) const
{
	return fontMetrics().width(string);
}

int LocalConfig::charWidth(int num) const
{
	return global_->config().charWidth(num);
}

int LocalConfig::charHeight() const
{
	return global_->config().charHeight();
}

int LocalConfig::byteHeight() const
{
	return global_->config().byteHeight();
}

const QFontMetrics &LocalConfig::fontMetrics() const
{
	return global_->config().fontMetrics();
}

int LocalConfig::top() const
{
	return global_->config().top();
}

Color::ColorContainer &LocalConfig::color()
{
	return colors_;
}

QColor LocalConfig::color(const QString &key) const
{
	Color::ColorContainer::const_iterator it = colors_.find(key);
	if (it != colors_.end()) {
		return it.value();
	} else {
		return global_->config().color(key);
	}
}

void LocalConfig::setColor(const QString &key, const QColor &color)
{
	colors_.insert(key, color);
}

}	// namespace Standard
