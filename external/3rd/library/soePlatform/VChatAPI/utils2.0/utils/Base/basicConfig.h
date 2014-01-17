#ifndef SERVER__BASIC_CONFIG_H
#define SERVER__BASIC_CONFIG_H

#include <string>
#include <map>
#include <vector>

#include "Api/apiTypeNameValuePair.h"
#include "stringutils.h"

class BasicConfig
{
    public:
		typedef std::pair<std::string,int> ConfigPair_t;
		typedef std::map<soe::lowerCaseString, ConfigPair_t> ConfigMap_t;

        BasicConfig();
		virtual ~BasicConfig();
        
		void			Push(const soe::NameValuePairs_t & paramValuePairs);
		void			Pull(soe::NameValuePairs_t & paramValuePairs) const;
        void			Load(const std::string & configFile);
        void			Set(const std::string & param, int value);
        void			Set(const std::string & param, const std::string & value);
        int				GetNumber(const std::string & param);
        std::string		GetString(const std::string & param);
        bool			IsSet(const std::string & param);
		std::string		GetConfigFileName() const { return mConfigFileName; }

		virtual	void	OnLoad();

    protected:
        ConfigMap_t		mParamMap;
		std::string		mConfigFileName;	
	private:
        char * pConfig;     // pointer to file memory
        char * pCursor;    // current cursor into file memory
};


#endif


