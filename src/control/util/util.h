
#pragma once

namespace util {

	// 
	template <typename T, size_t N> size_t countof(const T (&)[N])
	{
		return N;
	}

	static char itohex(int i)
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


