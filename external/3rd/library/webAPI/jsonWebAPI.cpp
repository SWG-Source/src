/*
 * Version: 1.75
 *
 * This code is just a simple wrapper around nlohmann's wonderful json lib
 * (https://github.com/nlohmann/json) and libcurl. While originally included directly,
 * we have come to realize that we may require web API functionality elsewhere in the future.
 *
 * As such, and in an effort to keep the code clean, we've broken it out into this simple little
 * namespace/lib that is easy to include. Just make sure to link against curl when including, and
 * make all the cmake modifications required to properly use it.
 *
 * (c) DarthArgus
 * based on the original prototype by parz1val
 *
 * License: LGPL, don't be a dick please
 */

#include "jsonWebAPI.h"

using namespace StellaBellum;

webAPI::webAPI(std::string endpoint, std::string userAgent) : uri(endpoint), userAgent(userAgent), statusCode(0) {}

webAPI::~webAPI() {
    requestData.clear();
    responseData.clear();
}

bool webAPI::setEndpoint(const std::string endpoint) {
    uri = endpoint;

    return true;
}

std::string webAPI::getRaw() {
    return sResponse;
}

bool webAPI::setData(std::string &data) {
    if (!data.empty()) {
        sRequest = data;

        return true;
    }

    return false;
}

std::string webAPI::getString(const std::string &slot) {
    if (!responseData.empty() && !slot.empty() && responseData.count(slot) && !responseData[slot].is_null()) {
        return responseData[slot].get<std::string>();
    }

    return std::string("");
}

std::unordered_map<int, std::string> webAPI::getStringMap(const std::string &slot) {
    std::unordered_map<int, std::string> ret = std::unordered_map<int, std::string>();

    if (!responseData.empty() && !slot.empty() && responseData.count(slot) && !responseData[slot].is_null()) {

        nlohmann::json j = responseData[slot];

        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
            int k = std::stoi(it.key());
            std::string val = it.value();

            ret.insert({k, val});
        }
    }

    return ret;
}

bool webAPI::submit(const int &reqType, const int &getPost, const int &respType) {
    if (reqType == DTYPE::JSON) // json request
    {
        if (!requestData.empty()) {
            // serialize our data into sRequest
            sRequest = requestData.dump();

            // clear our the object for next time
            requestData.clear();
        }
    }

    if (fetch(getPost, respType) && !(sResponse.empty())) {
        return true;
    }

    sResponse.clear();

    return false;
}

bool webAPI::fetch(const int &getPost, const int &mimeType) // 0 for json 1 for string
{
    bool fetchStatus = false;

    if (!uri.empty()) //data is allowed to be an empty string if we're doing a normal GET
    {
        CURL *curl = curl_easy_init(); // start up curl

        if (curl) {
            std::string readBuffer = ""; // container for the remote response
            struct curl_slist *slist = nullptr;

            // set the content type
            if (mimeType == DTYPE::JSON) {
                slist = curl_slist_append(slist, "Accept: application/json");
                slist = curl_slist_append(slist, "Content-Type: application/json");
            } else {
                slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
            }

            slist = curl_slist_append(slist, "charsets: utf-8");

            CURLcode res = curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
            res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback); // place the data into readBuffer using writeCallback
            res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // specify readBuffer as the container for data
            res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

            switch (getPost) {
                case HTTP::GET:
                    res = curl_easy_setopt(curl, CURLOPT_URL, std::string(uri + "?" + sRequest).c_str());
                    break;
                case HTTP::POST:
                    res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sRequest.c_str());
                    res = curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
                    break;
                    // want to do a put, or whatever other type? feel free to add here
            }

			// I suggest leaving VERIFYPEER = 0 because system SSL stores tend to be outdated 
            //if (uri.find(vxENCRYPT("stellabellum").decrypt()) != std::string::npos) {
                // the public one will verify but since this is pinned we don't care about the CA
                // to grab/generate, see https://curl.haxx.se/libcurl/c/CURLOPT_PINNEDPUBLICKEY.html
                // under the PUBLIC KEY EXTRACTION heading
                res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

				// if you want to pin to your own cert or cloudflares, learn how and use the below
                // res = curl_easy_setopt(curl, CURLOPT_PINNEDPUBLICKEY, vxENCRYPT("sha256//YOURKEYHERE").decrypt());
            //}

            if (res == CURLE_OK) {
                res = curl_easy_perform(curl); // make the request!
            }

            if (res == CURLE_OK) {
                char *contentType;

                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode); //get status code
                curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType); // get response mime type

                std::string conType(contentType);

                if (statusCode == 200 && !(readBuffer.empty())) // check it all out and parse
                {
                    sResponse = readBuffer;
                    if (conType.find("application/json") != std::string::npos) {
                        fetchStatus = processJSON();
                    } else {
                        responseData.clear();
                        fetchStatus = true;
                    }
                }
            }

            curl_slist_free_all(slist);
            curl_easy_cleanup(curl); // always wipe our butt
        }
    }

    if (!fetchStatus) {
        sResponse.clear();
        responseData.clear();
    }

    return fetchStatus;
}

// This is used by curl to grab the response and put it into a var
size_t webAPI::writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *) userp)->append((char *) contents, size * nmemb);
    return size * nmemb;
}

bool webAPI::processJSON() {
    if (!(sResponse.empty())) // check it all out and parse
    {
        try {
            responseData = nlohmann::json::parse(sResponse);
            return true;
        } catch (std::string &e) {
            responseData["message"] = e;
            responseData["status"] = "failure";
        } catch (...) {
            responseData["message"] = "JSON parse error for endpoint.";
            responseData["status"] = "failure";
        }
    } else {
        responseData["message"] = "Error fetching data from remote.";
        responseData["status"] = "failure";
    }

    return false;
}