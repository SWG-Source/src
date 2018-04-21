/*
This code is just a simple wrapper around nlohmann's wonderful json lib
(https://github.com/nlohmann/json) and libcurl. While originally included directly,
we have come to realize that we may require web API functionality elsewhere in the future.

As such, and in an effort to keep the code clean, we've broken it out into this simple little
namespace/lib that is easy to include. Just make sure to link against curl when including, and
make all the cmake modifications required to properly use it.

(c) stellabellum/swgilluminati (combined crews), written by DA with help from DC
based on the original prototype by parz1val

License: what's a license? we're a bunch of dirty pirates!
*/

#ifndef webAPI_H
#define webAPI_H

#include "json.hpp"
#include <curl/curl.h>

namespace webAPI
{
	using namespace std;
	
	string simplePost(string endpoint, string data, string slotName);
	//std::string simpleGet(char* endpoint, char* data);
	//nlohmann::json post(char* endpoint, char* data);
	//nlohmann::json get(char* endpoint, char* data);
	
	nlohmann::json request(string endpoint, string data, int reqType); // 1 for post, 0 for get
	size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
};

#endif
