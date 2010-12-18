
#pragma once

#include <QKeyEvent>

class Document;


class KeyboardHandler
{
protected:
	Document *document_;

public:
	KeyboardHandler(Document *document);
	virtual ~KeyboardHandler();

	virtual void keyPressEvent(QKeyEvent *);
	virtual void keyPressEvent(QChar ch);

protected:

	void changeData(quint64 pos, quint64 len, const uchar *data, uint data_len);
	void insertData(quint64 pos, uchar character);
	void insertData(quint64 pos, const uchar *data, uint len);
	void removeData(quint64 pos, quint64 len);

};


