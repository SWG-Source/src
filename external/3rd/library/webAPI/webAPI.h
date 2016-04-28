#ifndef webAPI_H
#define webAPI_H

#include "json.hpp"
#include <curl/curl.h>

namespace webAPI
{
	std::string simplePost(std::string endpoint, std::string data, std::string slotName);
	//std::string simpleGet(char* endpoint, char* data);
	//nlohmann::json post(char* endpoint, char* data);
	//nlohmann::json get(char* endpoint, char* data);
	
	nlohmann::json request(std::string endpoint, std::string data, int reqType); // 1 for post, 0 for get
	size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
};

#endif
