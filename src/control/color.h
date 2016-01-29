#pragma once

#include <QColor>
#include <QHash>

namespace Color {
  extern const QString kBackground;
  extern const QString kText;
  extern const QString kSelectBackground;
  extern const QString kSelectText;
  extern const QString kCaretBackground;

  typedef QHash<QString, QColor> ColorContainer;
}


struct ColorType {
  QString Background;
  QString Text;
  ColorType(const QString &bg_color = Color::kBackground, const QString &text_color = Color::kText)
  {
    Background = bg_color;
    Text = text_color;
  }
};
