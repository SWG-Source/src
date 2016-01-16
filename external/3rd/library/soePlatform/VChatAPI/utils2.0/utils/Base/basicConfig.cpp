#ifdef WIN32
#pragma warning( disable: 4786 )
#endif

#include <cstdlib>
#include <cstdio>
#include "basicConfig.h"
#include "profile.h"

#ifdef WIN32
#define snprintf _snprintf
#endif


using namespace std;

////////////////////////////////////////////////////////////////////////////////


BasicConfig::BasicConfig()
	: mParamMap()
	, mConfigFileName()
{
}

BasicConfig::~BasicConfig()
{
}

void BasicConfig::Push(const soe::NameValuePairs_t & paramValuePairs)
{
	for (soe::NameValuePairs_t::const_iterator pIter = paramValuePairs.begin(); pIter != paramValuePairs.end(); pIter++)
	{
		Set(pIter->name, pIter->value);
	}
}

void BasicConfig::Pull(soe::NameValuePairs_t & paramValuePairs) const
{
	size_t index = paramValuePairs.size();

	paramValuePairs.resize(index + mParamMap.size());
	for (ConfigMap_t::const_iterator pIter = mParamMap.begin(); pIter != mParamMap.end(); pIter++, index++)
	{
		paramValuePairs[index].name = pIter->first;
		paramValuePairs[index].value = pIter->second.first;
	}
}

void BasicConfig::Load(const std::string & configFile)
{
    static const char * config_delimiters1 = " \t\n:[]=";
    static const char * config_delimiters2 = "\t\n\"#";

    Profile profile("BasicConfig::Load()");
    FILE * file;
    if (!(file = fopen(configFile.c_str(), "rt")))
        return;

    char buffer[1024];
    while(fgets(buffer, 1024, file))
    {
        char * key = 0;
        char * value = 0;

        //  read key and value off line
        key = strtok(buffer, config_delimiters1);
        if (!key || key[0] == '#')
            continue;
        if (key)
        {
            value = key + strlen(key) + 1;
            while (*value == ' ' || *value == '\t')
                value++;
            value = strtok(value, config_delimiters2);
            if (value)
            {
                while (value[strlen(value)-1] == ' ')
                    value[strlen(value)-1] = 0;
            }
        }   

        //  insert key/value pair into the param map
        if (value)
			mParamMap[key] = ConfigPair_t(value, atoi(value));
        else
            mParamMap[key] = ConfigPair_t("", 0);
    }

    mConfigFileName = configFile;

    fclose(file);

    OnLoad();
}

void BasicConfig::Set(const std::string & param, int value)
{
    //  convert integer to string
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d", value);

    mParamMap[param] = ConfigPair_t(buffer, value);
}

void BasicConfig::Set(const std::string & param, const std::string & value)
{
    mParamMap[param] = ConfigPair_t(value, atoi(value.c_str()));
}

int BasicConfig::GetNumber(const std::string & param)
{
    return mParamMap[param].second;
}

std::string BasicConfig::GetString(const std::string & param)
{
    return mParamMap[param].first;
}

bool BasicConfig::IsSet(const std::string & param)
{
    return mParamMap.find(param) != mParamMap.end();
}

void BasicConfig::OnLoad()
{
}

