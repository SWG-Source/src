

namespace EnvironmentVariable
{
	//This function takes a string in the form key=value.  It will add
	//value to the environment variable key if key exists.  Otherwise it
	//will create a new env called key and set it to value.
	bool addToEnvironmentVariable(const char* key, const char* value);
	bool setEnvironmentVariable(const char* key, const char* value);
};
