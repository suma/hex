#pragma once

#include "global.h"
#include "../color.h"

namespace Standard {

class LocalConfig
{
public:
  LocalConfig(Global *global);
  virtual ~LocalConfig();

  const QFont &font() const;

  Color::ColorContainer &color();
  QColor color(const QString &key) const;
  void setColor(const QString &key, const QColor &color);

  uint num() const;

  void updateFont();
  int textWidth(const QString &string) const;
  int charWidth(int num = 1) const;
  int charHeight() const;
  int byteHeight() const;
  const QFontMetrics &fontMetrics() const;
  int top() const;

protected:
    Global *global_;
    Color::ColorContainer colors_;

};

}  // namespace Standard
