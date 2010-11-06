

#include "textdecodehelper.h"

namespace Standard {

class ShiftJISDecoder : public TextDecoder
{
private:
	bool isLeadByte(uchar c) const
	{
		return (0x81 <= c && c <= 0x9F) || (0xE0 <= c && c <= 0xEF);
	}

	bool isTailByte(uchar c) const
	{
		return (0x40 <= c && c <= 0xE7) || (0x80 <= c && c <= 0xFC);
	}

	bool isASCIISpace(uchar c) const
	{
		return c == 0x20;
	}

	bool isASCIINumeric(uchar c) const
	{
		return 0x30 <= c && c <= 0x39;
	}

	bool isASCIISymbol(uchar c) const
	{
		return (0x21 <= c && c <= 0x2F) ||
			(0x3A <= c && c <= 0x40) ||
			(0x5B <= c && c <= 0x60) ||
			(0x7B <= c && c <= 0x7E);
	}

	bool isASCIIAlphabet(uchar c) const
	{
		return (0x41 <= c && c <= 0x5A) || (0x61 <= c && c <= 0x7A);
	}

	bool isASCII(uchar c) const
	{
		return isASCIISpace(c) || isASCIINumeric(c) || isASCIISymbol(c) || isASCIIAlphabet(c);
	}

public:

	// relative pos from Top
	uint getPrintableBytes(const Document &document, uint position)
	{
		uchar data[32];
		uint len = (uint)qMin((quint64)32, document.length() - position);

		document.get(position, data, len);


		// ここからチェック
		if (len >= 2 && isLeadByte(data[0]) && isTailByte(data[1])) {
			return 2;
		}
		if (isASCII(data[0])) {
			return 1;
		}

		return 0;
	}
};



TextDecodeHelper::TextDecodeHelper(Document &doc, QString textEncoding, quint64 top)
	: document_(doc)
	, textEncoding_(textEncoding)
	, top_(top)
	, posMax_(0)
	, codec_(NULL)
	, decoder_(NULL)
{
	initCodec();

	decoder_ = new ShiftJISDecoder();
}

void TextDecodeHelper::initCodec()
{
	codec_ = QTextCodec::codecForName("Shift-JIS");
}

const QTextCodec* TextDecodeHelper::getCodec() const
{
	return codec_;
}



// relative pos from Top
uint TextDecodeHelper::getPrintableBytes(uint pos) const
{
	quint64 position = top_ + pos;
	Q_ASSERT(position <= document_.length());

	return decoder_->getPrintableBytes(document_, position);
}


}	// namespace
