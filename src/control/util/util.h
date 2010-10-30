#ifndef UTIL_H_INC
#define UTIL_H_INC

namespace util {

	// 
	template <typename T, size_t N> size_t countof(const T (&)[N])
	{
		return N;
	}

}

#endif
