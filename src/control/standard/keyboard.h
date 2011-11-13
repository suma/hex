

#pragma once

#include <QtGlobal>
#include <vector>
#include "../keyboardhandler.h"

namespace Standard {

class Global;
class HexView;
class TextView;

class Keyboard : public ::KeyboardHandler
{
private:
	Global *global_;
	HexView *view_;

public:
	Keyboard(Global *global, HexView *view);
	~Keyboard();

	void keyPressEvent(QKeyEvent *);
	void keyInputEvent(QString str);
	void keyInputEvent(QChar ch);

	void home();
	void end();
	void left();
	void right();
	void up();
	void down();
	void pageUp();
	void pageDown();
	void scrollUp();
	void scrollDown();

};


}	// namespace

