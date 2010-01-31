#include <QApplication>
#include <QtDebug>
#include <iostream>
#include "document.h"
#include "control/standard/textview.h"
#include "control/standard/textcursor.h"
#include "control/standard/textdecodehelper.h"

using namespace Standard;

int main(int argc, char *argv[])
{
	// utf-8な文字列をQStringが受け付けるようにする。
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	// utf-8な文字列をtr()が受け付けるようにする。
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));

	printf("test\n");
	char data1[] = { 0x61, '\0' };	// 'a'
	char data2[] = { 0xC6, '\0' };	// 'ﾀ'
	char data3[] = { 0xFF, 'a', '\0' }; // . a .
	char data4[] = { 0, 0 };
	char data5[] = { 0x82, 0x50, 0 };

	char *data[] = {
		data1,
		data2,
		data3,
		data4,
		data5,
	};
	int data_len[] = {
		2,
		2,
		3,
		2,
		2,
	};

#if 0
	for (int i = 0; i < 4; i++) {
		printf("== Trial %d ==\n", i+1);
		printf(" target = %s\n", data[i]);

		Document doc;
		doc.insert(0, (uchar*)data[i], data_len[i]);

		TextDecodeHelper helper(&doc, QString(""), 0);
		helper.CheckTop(0);

		uint current = 0;
		uint next = helper.GetPrintableCharLength(current);
		uint char_length = helper.GetPrintableCharLength(next);

		printf("next: %u\n", next);
		printf("charlen: %u\n", char_length);
	}

#endif

	for (int i = 0; i < sizeof(data_len)/sizeof(*data_len); i++) {
		printf("== Trial %d ==\n", i+1);
		qDebug() << " target = " << data[i];
		qDebug() << " len =    " << data_len[i];
		//printf(" target = %s len = %d\n", data[i], data_len[i]);

		QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
		QTextCodec::ConverterState state(QTextCodec::ConvertInvalidToNull);

		int start = 0;
		while (start < data_len[i]) {
			qDebug("i=%d, Start = %d", i, start);
			QString s = codec->toUnicode((char*)data[i] + start, 1, &state);
			qDebug("[%d] invalid %d", start, state.invalidChars);
			qDebug("[%d] remaining %d", start, state.remainingChars);
			qDebug("[%d] S.size = %d", start, s.size());
			qDebug("[%d] S = ", start);
			for (int x = 0; x < s.size(); x++) {
				if (s[x] == QChar('\0')) {
					qDebug() << QString(" null ");
				} else {
					qDebug() << s[x];
				}
			}
			start++;
		}

	}

	return 0;
}

