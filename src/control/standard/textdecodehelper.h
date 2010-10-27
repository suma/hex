
#pragma once

#include <algorithm>
#include <deque>
#include <vector>
#include <QTextCodec>

#include "../document.h"

namespace Standard {

class TextDecoder
{
public:
	virtual uint getPrintableBytes(const Document &doc, uint pos) = 0;
};


class TextDecodeHelper
{
private:
	Document &document_;
	QString textEncoding_;
	quint64 top_;	// Start line to draw text じゃなくて positionに
	std::deque<uint> positions_;
	uint posMax_;
	std::vector<uchar> buff_;
	QTextCodec *codec_;

	TextDecoder *decoder_;
public:
	TextDecodeHelper(Document &doc, QString textEncoding, quint64 top);

	void initCodec();
	const QTextCodec* getCodec() const;


	// relative pos from Top
	uint getPrintableBytes(uint pos) const;


};

}	// namespace

