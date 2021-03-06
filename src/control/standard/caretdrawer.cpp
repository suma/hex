#include "textview.h"
#include <QWidget>
#include "caretdrawer.h"
#include "cursor.h"
#include "../document.h"

namespace Standard {

CaretDrawer::CaretDrawer(QWidget *parent, Caret &caret)
  : QObject(parent)
  , caret_(caret)
{
  //setFocusPolicy(Qt::NoFocus);
  enable();
}


CaretDrawer::~CaretDrawer()
{
}

void CaretDrawer::enable()
{
  setCaretBlink(true);
  caret_.setEnable();
}

void CaretDrawer::disable()
{
  setCaretBlink(false);
  if (!caret_.visible()) {
    caret_.setVisible(true);
    qobject_cast<QWidget*>(parent())->update();
  }
}

void CaretDrawer::setCaretBlink(bool enable)
{
  if (!caret_.enable() || !caret_.blinkTime()) {
    return;
  }
  const int timer = caret_.timerId();
  if (enable) {
    if (timer == 0) {
      caret_.setTimerId(startTimer(caret_.blinkTime()));
    }
  } else {
    if (timer != 0) {
      killTimer(timer);
      caret_.setTimerId(0);
    }
  }
}

void CaretDrawer::timerEvent(QTimerEvent *event)
{
  if (caret_.timerId() == event->timerId()) {
    caret_.inverseVisible();
    qobject_cast<QWidget*>(parent())->update();
  }
}

TextCaretDrawer::TextCaretDrawer(QWidget *parent, TextConfig &config, Cursor *cursor, Caret &caret, ::Document *document)
  : CaretDrawer(parent, caret)
  , config_(config)
  , cursor_(cursor)
  , document_(document)
{
}

TextCaretDrawer::~TextCaretDrawer()
{
}

void TextCaretDrawer::paintEvent(QPaintEvent *event)
{
  if (!caret_.enable() || !caret_.visible()) {
    return;
  }

  // Get caret coordinates
  const quint64 pos = cursor_->position();
  const int x = pos % config_.num();
  const int y = config_.top() + config_.byteHeight() * (pos / config_.num() - cursor_->top());

  const bool caret_middle = pos < document_->length();

  // FIXME
  QRect update_area(config_.x(x), y, config_.charWidth(2), config_.byteHeight());

  if (!event->region().contains(update_area)) {
    return;
  }

  QPainter painter(qobject_cast<QWidget*>(parent()));
  painter.setFont(config_.font());
  CaretDrawInfo info(painter, caret_.currentShape(), pos, x, y, caret_middle);
  drawCaret(info);
}

void TextCaretDrawer::drawCaret(CaretDrawInfo info)
{
  switch (info.shape) {
  case CARET_LINE:
    drawCaretLine(info);
    break;
  case CARET_BLOCK:
    drawCaretBlock(info);
    break;
  case CARET_FRAME:
    drawCaretFrame(info);
    break;
  case CARET_UNDERBAR:
    drawCaretUnderbar(info);
    break;
  default:
    ;
  }
}

// FIXME:  caret width(for Multibyte Character)

void TextCaretDrawer::drawCaretLine(const CaretDrawInfo &info)
{
  int x;
  if (true) {//if (!info.caret_middle) {
    x = config_.x(info.x);
  } else {
    x = config_.x(info.x) + config_.charWidth();
  }
  QBrush brush(config_.color(Color::kCaretBackground));
  info.painter.fillRect(x, info.y, 2, config_.byteHeight(), brush);
}

void TextCaretDrawer::drawCaretBlock(const CaretDrawInfo &info)
{
  QBrush brush(config_.color(Color::kCaretBackground));
  if (info.caret_middle) {
    info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
    // TODO: 本当はここで文字描画
  } else {
    // Draw block without data
    info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
  }
}

void TextCaretDrawer::drawCaretFrame(const CaretDrawInfo &info)
{
  int width = config_.byteWidth() - 1;
  int x = config_.x(info.x);

  info.painter.setPen(config_.color(Color::kCaretBackground));
  info.painter.drawRect(x, info.y, width, config_.byteHeight() - 1);
}

void TextCaretDrawer::drawCaretUnderbar(const CaretDrawInfo &info)
{
  int width = config_.byteWidth() - 1;
  int x = config_.x(info.x);

  QBrush brush(config_.color(Color::kCaretBackground));
  info.painter.fillRect(x, info.y + config_.byteHeight() - 2, width, 2, brush);
}



HexCaretDrawer::HexCaretDrawer(QWidget *parent, HexConfig &config, Cursor *cursor, Caret &caret, ::Document *document)
  : CaretDrawer(parent, caret)
  , config_(config)
  , cursor_(cursor)
  , document_(document)
{
}

HexCaretDrawer::~HexCaretDrawer()
{
}

void HexCaretDrawer::paintEvent(QPaintEvent *event)
{
  if (!caret_.enable() || !caret_.visible()) {
    return;
  }

  // Get caret coordinates
  const quint64 pos = cursor_->position();
  const int x = pos % config_.num();
  const int y = config_.top() + config_.byteHeight() * (pos / config_.num() - cursor_->top());

  const bool caret_middle = pos < document_->length();

  // FIXME
  QRect update_area(config_.x(x), y, config_.charWidth(2), config_.byteHeight());

  if (!event->region().contains(update_area)) {
    return;
  }


  QPainter painter(qobject_cast<QWidget*>(parent()));
  painter.setFont(config_.font());
  CaretDrawInfo info(painter, caret_.currentShape(), pos, x, y, caret_middle);
  drawCaret(info);
}

void HexCaretDrawer::drawCaret(CaretDrawInfo info)
{
  switch (info.shape) {
  case CARET_LINE:
    drawCaretLine(info);
    break;
  case CARET_BLOCK:
    drawCaretBlock(info);
    break;
  case CARET_FRAME:
    drawCaretFrame(info);
    break;
  case CARET_UNDERBAR:
    drawCaretUnderbar(info);
    break;
  default:
    ;
  }
}

void HexCaretDrawer::drawCaretLine(const CaretDrawInfo &info)
{
  int x;
  if (cursor_->nibble() || !info.caret_middle) {
    x = config_.x(info.x);
  } else {
    x = config_.x(info.x) + config_.byteMargin().left() + config_.charWidth();
  }
  QBrush brush(config_.color(Color::kCaretBackground));
  info.painter.fillRect(x, info.y, 2, config_.byteHeight(), brush);
}

void HexCaretDrawer::drawCaretBlock(const CaretDrawInfo &info)
{
  QBrush brush(config_.color(Color::kCaretBackground));
  ColorType color = cursor_->getSelection().color(info.pos, caret_);
  
  if (info.caret_middle) {
    if (cursor_->nibble() || cursor_->hasSelection()) {
      // Draw block byte
      info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
    } else {
      // Draw block lowwer nibble
      info.painter.fillRect(config_.x(info.x) + config_.byteMargin().left() + config_.charWidth(), info.y, config_.charWidth() + config_.byteMargin().right(), config_.byteHeight(), brush);
    }
  } else {
    // Draw block without data
    info.painter.fillRect(config_.x(info.x), info.y, config_.byteWidth(), config_.byteHeight(), brush);
  }
}

void HexCaretDrawer::drawCaretFrame(const CaretDrawInfo &info)
{
  int width, x;
  if (cursor_->nibble() || !info.caret_middle) {
    width = config_.byteWidth() - 1;
    x = config_.x(info.x);
  } else {
    width = config_.charWidth() + config_.byteMargin().right() - 1;
    x = config_.x(info.x) + config_.charWidth() + config_.byteMargin().left();
  }
  info.painter.setPen(config_.color(Color::kCaretBackground));
  info.painter.drawRect(x, info.y, width, config_.byteHeight() - 1);
}

void HexCaretDrawer::drawCaretUnderbar(const CaretDrawInfo &info)
{
  int width, x;
  if (cursor_->nibble() || !info.caret_middle) {
    width = config_.byteWidth() - 1;
    x = config_.x(info.x);
  } else {
    width = config_.charWidth() + config_.byteMargin().right() - 1;
    x = config_.x(info.x) + config_.byteMargin().left() + config_.charWidth();
  }

  QBrush brush(config_.color(Color::kCaretBackground));
  info.painter.fillRect(x, info.y + config_.byteHeight() - 2, width, 2, brush);
}




}  // namespace
