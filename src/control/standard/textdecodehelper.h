
#pragma once

#include <algorithm>
#include <deque>
#include <vector>
#include <QTextCodec>
#include <QTextDecoder>

/*
 * 
 *
 */

namespace Standard {

class TextDecodeHelper
{
private:
	Document *document_;
	QString textEncoding_;
	quint64 top_;	// Start line to draw text じゃなくて positionに
	std::deque<uint> positions_;
	uint posMax_;
	std::vector<uchar> buff_;
	QTextCodec *codec_;
	QTextDecoder *decoder_;
public:
	TextDecodeHelper(Document *doc, QString textEncoding, quint64 top)
		: document_(doc)
		, textEncoding_(textEncoding)
		, top_(top)
		, posMax_(0)
		, codec_(NULL)
		, decoder_(NULL)
	{
		initCodec();
	}

	void initCodec()
	{
		codec_ = QTextCodec::codecForName("Shift-JIS");
	}

	const QTextCodec* getCodec() const
	{
		return codec_;
	}

	void CheckTop(quint64 top)
	{
		if (top_ != top) {
			flush(top);
		}
	}

	void flush(quint64 top)
	{
		top_ = top;
		positions_.clear();
	}

	uint GetStartPosition(uint pos) const
	{
		if (posMax_ <= pos) {
			return pos;
		}
		return *(lower_bound(positions_.begin(), positions_.end(), pos));
	}

	void AppendPosition(uint pos)
	{
		posMax_ = qMax(pos, posMax_);
		std::deque<uint>::iterator it = lower_bound(positions_.begin(), positions_.end(), pos);
		if (*it != pos) {
			positions_.insert(it, pos);
		}
	}

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

	// relative pos from Top
	uint get_printable_bytes(uint pos)
	{
		quint64 position = top_ + pos;
		Q_ASSERT(position <= document_->length());

		uchar data[32];
		uint len = (uint)qMin((quint64)32, document_->length() - position);

		document_->get(position, data, len);


		// ここからチェック
		if (len >= 2 && isLeadByte(data[0]) && isTailByte(data[1])) {
			return 2;
		}
		if (isASCII(data[0])) {
			return 1;
		}

		return 0;
#if 0
		quint64 index = top * HexConfig::Num + pos;
		uint size = qMin(document_->length() - index, (quint64)32) & 0xFF;
		if (buff_.capacity() < size) {
			buff_.resize(size);
		}
		uchar *b = &buff_[0];
		document_->get(index, b, size);

		QTextCodec::ConverterState state(QTextCodec::ConvertInvalidToNull);


		int start = 2;
		while (true) {
			QString s = codec_->toUnicode((char*)b, start, &state);
			qDebug("[%d] invalid %d", start, state.invalidChars);
			qDebug("[%d] remaining %d", start, state.remainingChars);
			qDebug("[%d] S.size = %d", start, s.size());
			qDebug("[%d] S = %s", start, s.toStdString().c_str());
			if (s[0] == QChar('\0')) {
				qDebug("[%d] S[0] is null", start);
			}
			if (state.remainingChars <= 1) {
				break;
			}
		}
#endif

		//return size - state.remainingChars;
#if 0
		int res = 0;
		qDebug("s.size: %d", s.size());
		for (int i = 0; i < s.size(); i++, res++) {
			if (s[i] == QChar('\0')) {
				qDebug("[%d] = null", i);
				break;
			}
		}
		qDebug("str size: %u",  size - state.remainingChars);
#endif
		
		return 0;
	}

	uint GetNextPrintableCharBytes(uint pos) const
	{
		return 0;
	}

};

}	// namespace

