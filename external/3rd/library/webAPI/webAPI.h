/*
 * Version: 1.3
 * 
 * This code is just a simple wrapper around nlohmann's wonderful json lib
 * (https://github.com/nlohmann/json) and libcurl. While originally included directly,
 * we have come to realize that we may require web API functionality elsewhere in the future.
 * 
 * As such, and in an effort to keep the code clean, we've broken it out into this simple little
 * namespace/lib that is easy to include. Just make sure to link against curl when including, and
 * make all the cmake modifications required to properly use it.
 * 
 * (c) stellabellum/swgilluminati (combined crews), written by DA with help from DC
 * based on the original prototype by parz1val
 * 
 * License: what's a license? we're a bunch of dirty pirates!
 */

#ifndef webAPI_H
#define webAPI_H

#include "json.hpp"

#ifdef WIN32
#include <curl.h>
#else
#include <curl/curl.h>
#endif

namespace StellaBellum
{
	class webAPI
	{
	public:    
		// useragent
		std::string userAgent;
		
		// constructor - can setup with the endpoint from the start
		webAPI(std::string endpoint, std::string userAgent = "StellaBellum webAPI");
		~webAPI();
		
		// submits the request - respType 0 is json, else raw, get = 0 post = 1
		bool submit(const int &reqType = 0, const int &getPost = 1, const int &respType = 0);
		
		// set the endpoint after object creation...or change the target if needed
		bool setEndpoint(std::string endpoint);
		
		// get raw response
		std::string getRaw();
		
		// set a standard request string
		bool setData(std::string &data); // all or nothing
		
		// set json key and value for request
		template<typename T> bool addJsonData(std::string key, T value) 
		{ 
			if (!key.empty())
			{
				requestData[key] = value;	
				return true;
			}
			
			return false;
		}
		
		// get json response slot
		template<typename T> T getRespValue(std::string slot) 
		{ 
			if (!responseData.is_null() && !slot.empty() && responseData.count(slot))
			{
				return responseData[slot].get<T>();
			}
			
			return nullptr;
		}
		
	private:
		// json request data - object is serialized before sending, used with above setter template
		nlohmann::json requestData;
		
		// json response, stored so we can use the getter template above
		nlohmann::json responseData;
		
		// raw response
		std::string sResponse;
		
		// raw request string
		std::string sRequest;
		
		// API endpoint
		std::string uri;
		
		// fetcher - returns raw response direct from remote - 0 for get, 1 for post, 0 for json mime, 1 for standard
		bool fetch(const int &getPost = 1, const int &mimeType = 0);
		
		// cURL writeback callback
		static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
		
		// json processor - string to json
		bool processJSON();
		
	protected:
		// http response code (200, 404, etc)	 
		long statusCode;
	};
}
#endif
