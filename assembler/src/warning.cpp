#include "./warning.h"

namespace Warning{
	uint64_t enabled  = ~0;
	uint64_t promoted =  0;

	bool is_error(Warning::id const w)
	{
		return !!(Warning::promoted & w);
	}
	bool is_warn(Warning::id const w)
	{
		return !!(Warning::enabled  & w);
	}
}
