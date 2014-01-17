
#include "FirstTaskManager.h"

#include <stdlib.h>
#include <string>

namespace EnvironmentVariable
{
	bool addToEnvironmentVariable(const char* key, const char* value)
	{
		bool retval = false;
		const char* oldValue = getenv(key);
		if (oldValue)
		{
			std::string s(oldValue);
			s += ":";
			s += value;
			
			//Bad things happen if the first character happens to be : (ie from an empty environment string)
			const char* newValue = s.c_str();
			if (newValue[0] == ':')
				++newValue;
			
			retval = (setenv(key, newValue, 1) == 0);
		}
		else
		{
			retval = (setenv(key, value, 0) == 0);
		}
		return retval;
	}


	bool setEnvironmentVariable(const char* key, const char* value)
	{
		return (setenv(key, value, 1) == 0);
	}
	
};
