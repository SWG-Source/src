
#include "webAPI.h"

// if status == success, returns "success", or slotName's contents if specified...
// otherwise returns the "message" if no success
std::string simplePost(std::string endpoint, std::string data, std::string slotName)
{
	nlohmann::json response = request(endpoint, data, 1);
	std::string output;

	if (response.count("status") && response["status"].get<std::string>() == "success")
	{
		if (slotName && response.count(slotName))
		{
			output = response[slotName].get<std::string>();
		}
		else
		{
			output = "success";
		}
	}
	else
	{
		if (j.count("message"))
		{
			output = j["message"].get<std::string>();
		}
		else
		{
			output = "Message not provided by authentication service.";
		}
	}

	return output;				
}

nlohmann::json request(std::string endpoint, std::string data, int reqType, ...)
{
	nlohmann::json response;

	if (!endpoint.empty()) //data is allowed to be an empty string if we're doing a normal GET
	{
		CURL *curl = curl_easy_init();

                if (curl)
                {
                        std::string readBuffer;

                        curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);			

			if (reqType == 1)
			{
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
			}

			CURLcode res = curl_easy_perform(curl);

			if (res == CURLE_OK && !(readBuffer.empty()))
			{
                        	response = json::parse(readBuffer);
			}
			curl_easy_cleanup(curl); 
		}
		else
		{
        		response["message"] = "Failed to initialize cURL.";
        		response["status"] = "failure";
		}
	}
	else
	{
        	response["message"] = "Invalid endpoint URL.";
        	response["status"] = "failure";
	}

	return response;
}

// This is used by curl to grab the response and put it into a var
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
}
