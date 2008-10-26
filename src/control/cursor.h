#ifndef CURSOR_H_INC
#define CURSOR_H_INC

class Document;

class Cursor
{
protected:
	Document *doc_;

public:
	Cursor(Document *doc);
	virtual ~Cursor();

};



#endif
