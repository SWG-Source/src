/*
Version: 1.3

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

#include "webAPI.h"

using namespace StellaBellum;

webAPI::webAPI(std::string endpoint, std::string userAgent) : uri(endpoint), userAgent(userAgent) {}
webAPI::~webAPI()
{
	requestData.clear();
	responseData.clear();
}

bool webAPI::setEndpoint(std::string endpoint)
{
	uri = endpoint;
	
	return true;
}

std::string webAPI::getRaw()
{
	return sResponse;
}

bool webAPI::setData(std::string &data)
{
	if (!data.empty()) 
	{
		sRequest = data;
		
		return true;
	}
	
	return false;
}

bool webAPI::submit(const int &reqType, const int &getPost, const int &respType)
{	
	
	if (reqType == 0) // json request
	{
		if (!requestData.is_null()) 
		{
			// serialize our data into sRequest
			sRequest = requestData.dump();
			
			// clear our the object for next time
			requestData.clear();
		}
		else
		{
			return false;
		}
	}
	
	if (fetch(getPost, respType) && !(sResponse.empty())) 
	{
		return true;
	}
		
	sResponse.clear();
	
	return false;
}

bool webAPI::fetch(const int &getPost, const int &mimeType) // 0 for json 1 for string
{
	if (!uri.empty()) //data is allowed to be an empty string if we're doing a normal GET
	{
		CURL *curl = curl_easy_init(); // start up curl

		if (curl)
		{
			std::string readBuffer; // container for the remote response
			struct curl_slist *slist = nullptr;
			
			// set the content type
			if (mimeType == 0) 
			{
				slist = curl_slist_append(slist, "Content-Type: application/json");	
			}
			else
			{
				slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
			}
			
			curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback); // place the data into readBuffer using writeCallback
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // specify readBuffer as the container for data
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);		
			
			switch (getPost)
			{
			  case 0:
			    curl_easy_setopt(curl, CURLOPT_URL, std::string(uri + "?" + sRequest).c_str());
			    break;
			  case 1:
			    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sRequest.c_str());
			    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
			    break;
			  // want to do a put, or whatever other type? feel free to add here
			}

			CURLcode res = curl_easy_perform(curl); // make the request!
			char *contentType;
			
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode); //get status code
			curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType); // get response mime type
			
			curl_slist_free_all(slist);
			curl_easy_cleanup(curl); // always wipe our butt
			
			if (res == CURLE_OK && statusCode == 200 && !(readBuffer.empty())) // check it all out and parse
			{
				sResponse = readBuffer;
				
				if (std::string(contentType) == "application/json")
				{
					processJSON();
				}
				else
				{
					responseData.clear();
				}
				
				return true;
			}
		}
	}
	
	sResponse.clear();
	responseData.clear();
	
	return false;
}

// This is used by curl to grab the response and put it into a var
size_t webAPI::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

bool webAPI::processJSON()
{
	if (!(sResponse.empty())) // check it all out and parse
	{
		try 
		{
			responseData = nlohmann::json::parse(sResponse);
			return true;
		}
		catch (std::string &e) 
		{
			responseData["message"] = e;
			responseData["status"] = "failure";
		}
		catch (...) 
		{
			responseData["message"] = "JSON parse error for endpoint.";
			responseData["status"] = "failure";
		}
	}
	else
	{
		responseData["message"] = "Error fetching data from remote.";
		responseData["status"] = "failure";
	}
	
	return false;
}
