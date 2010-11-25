
#include <QtGlobal>

namespace util {

char itohex(uint i)
{
	// [0-9a-f]
	Q_ASSERT(i <= 15);

	if (i <= 9) {
		return '0' + i;
	} else {
		return 'A' + i - 10;
	}
}

}
