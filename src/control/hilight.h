#ifndef HILIGHT_H_INC
#define HILIGHT_H_INC

class Document;

class Hilight
{
protected:
	Document *doc_;

public:
	Hilight(Document *doc);
	~Hilight();

};



#endif
