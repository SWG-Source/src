#include "FirstTaskManager.h"

namespace EnvironmentVariable
{
	bool addToEnvironmentVariable(const char* key, const char* value)
	{
		bool retval = false;
		char oldValue[256];
		DWORD tmp = GetEnvironmentVariable(key, oldValue, sizeof(oldValue));
		if (tmp != 0)
		{
			std::string s(oldValue);
			s += ";";
			s += value;
			
			//Bad things happen if the first character happens to be ; (ie from an empty environment string)
			const char* newValue = s.c_str();
			if (newValue[0] == ';')
				++newValue;
			
			retval = (SetEnvironmentVariable(key, newValue) != 0);
		}
		else
		{
			retval = (SetEnvironmentVariable(key, value) != 0);
		}
		return retval;
	}
	bool setEnvironmentVariable(const char* key, const char* value)
	{
		return (SetEnvironmentVariable(key, value) != 0);
	}
};
