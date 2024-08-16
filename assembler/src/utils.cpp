#include "./utils.h"
#include "top/gen_utils/log.h"

namespace Utils{
	//wrappers for Log::warning respecting current settings
	void warning(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num,
		Warning::id      const wid,
		std::string_view const info
		)
	{
		     if(Warning::is_error(wid))
		{
			if(info.size() > 0) 
				Log::error(str, info, file_name, line_num);
			else
				Log::error(str,       file_name, line_num);
		}
		else if(Warning::is_warn (wid))
		{
			if(info.size() > 0) 
				Log::warning(str, info, file_name, line_num);
			else
				Log::warning(str,       file_name, line_num);
		}

		return;
	}
	void warning(
		std::string_view const str,
		Warning::id      const wid,
		std::string_view const info
		)
	{
		     if(Warning::is_error(wid))
		{
			Log::error  (str);
			if(info.size() > 0) Log::info(info);
		}
		else if(Warning::is_warn (wid))
		{
			Log::warning(str);
			if(info.size() > 0) Log::info(info);
		}

		return;
	}


	uint32_t digit_count(uint32_t x)
	{
		if (x >= 10000) {
			if (x >= 10000000) {
				if (x >= 100000000) {
					if (x >= 1000000000)
						return 10;
					return 9;
				}
				return 8;
			}
			if (x >= 100000) {
				if (x >= 1000000)
					return 7;
				return 6;
			}
			return 5;
		}
		if (x >= 100) {
			if (x >= 1000)
				return 4;
			return 3;
		}
		if (x >= 10)
			return 2;
		return 1;
	}

};
