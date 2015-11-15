#ifndef API_TYPE_NAME_VALUE_PAIR
#define API_TYPE_NAME_VALUE_PAIR

#include <climits>
#include <string>
#include <vector>
#include "Base/serializeClasses.h"

const int MAX_NAME_VALUE_PAIR_NAME_LEN  = 256;
const int MAX_NAME_VALUE_PAIR_VALUE_LEN = 512;

namespace soe
{
    struct NameValuePair
    {
		unsigned Write(unsigned char * stream, unsigned size) const
		{
			unsigned bytesTotal = 0;
			unsigned bytes = 0;

			bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, name));
			if (!bytes) return 0;

			bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, value));
			if (!bytes) return 0;

			return bytesTotal;
		}

		unsigned Read(const unsigned char * stream, unsigned size)
		{
			unsigned bytesTotal = 0;
			unsigned bytes = 0;

			bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, name, MAX_NAME_VALUE_PAIR_NAME_LEN));
			if (!bytes) return 0;

			bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, value, MAX_NAME_VALUE_PAIR_VALUE_LEN));
			if (!bytes) return 0;

			return bytesTotal;
		}

        std::string name; //256
        std::string value; //512
    };
    typedef std::vector<NameValuePair> NameValuePairs_t;

	inline unsigned Read(const unsigned char * stream, unsigned size, NameValuePair & data, unsigned maxLen, unsigned version = 0)
	{
		return data.Read(stream, size);
	}
	
	inline unsigned Write(unsigned char * stream, unsigned size, const NameValuePair & data, unsigned version = 0)
	{
		return data.Write(stream, size);
	}

	inline int Print(char * stream, unsigned size, const NameValuePair & data, unsigned maxDepth = INT_MAX)
	{
		return 0;
	}
}


#endif
