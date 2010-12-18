
#pragma once

namespace util {

	// 
	template <typename T, size_t N> size_t countof(const T (&)[N])
	{
		return N;
	}

	char itohex(uint i);
}


