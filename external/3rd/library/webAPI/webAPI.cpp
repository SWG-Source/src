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

#include "webAPI.h"

using namespace StellaBellum;

webAPI::webAPI(std::string endpoint, std::string userAgent) : uri(endpoint), userAgent(userAgent), statusCode(0) {}

webAPI::~webAPI() {
    this->requestData.clear();
    this->responseData.clear();
}

bool webAPI::setEndpoint(const std::string endpoint) {
    this->uri = endpoint;

    return true;
}

std::string webAPI::getRaw() {
    return this->sResponse;
}

bool webAPI::setData(std::string &data) {
    if (!data.empty()) {
        this->sRequest = data;

        return true;
    }

    return false;
}

std::string webAPI::getString(const std::string &slot) {
    if (!this->responseData.empty() && !slot.empty() && responseData.count(slot) &&
        !this->responseData[slot].is_null()) {
        return this->responseData[slot].get<std::string>();
    }

    return std::string("");
}

std::vector<std::string> getStringVector(const std::string &slot) {
    if (!this->responseData.empty() && !slot.empty() && responseData.count(slot) && !this->responseData[slot].is_null()) {
        return this->responseData[slot].get<std::vector<std::string>>();
    }

    return std::vector<std::string>();
}

bool webAPI::submit(const int &reqType, const int &getPost, const int &respType) {
    if (reqType == DTYPE::JSON) // json request
    {
        if (!this->requestData.empty()) {
            // serialize our data into sRequest
            this->sRequest = this->requestData.dump();

            // clear our the object for next time
            this->requestData.clear();
        }
    }

    if (fetch(getPost, respType) && !(this->sResponse.empty())) {
        return true;
    }

    this->sResponse.clear();

    return false;
}

bool webAPI::fetch(const int &getPost, const int &mimeType) // 0 for json 1 for string
{
    bool fetchStatus = false;

    if (!uri.empty()) //data is allowed to be an empty string if we're doing a normal GET
    {
        CURL *curl = curl_easy_init(); // start up curl

        if (curl) {
            std::string readBuffer; // container for the remote response
            struct curl_slist *slist = nullptr;

            // set the content type
            if (mimeType == DTYPE::JSON) {
                slist = curl_slist_append(slist, "Accept: application/json");
                slist = curl_slist_append(slist, "Content-Type: application/json");
            } else {
                slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
            }

            slist = curl_slist_append(slist, "charsets: utf-8");

            curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback); // place the data into readBuffer using writeCallback
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // specify readBuffer as the container for data
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

            switch (getPost) {
                case HTTP::GET:
                    curl_easy_setopt(curl, CURLOPT_URL, std::string(this->uri + "?" + sRequest).c_str());
                    break;
                case HTTP::POST:
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, this->sRequest.c_str());
                    curl_easy_setopt(curl, CURLOPT_URL, this->uri.c_str());
                    break;
                    // want to do a put, or whatever other type? feel free to add here
            }

            CURLcode res = curl_easy_perform(curl); // make the request!
            char *contentType;

            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &this->statusCode); //get status code
            curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType); // get response mime type

            std::string conType(contentType);

            if (res == CURLE_OK && this->statusCode == 200 && !(readBuffer.empty())) // check it all out and parse
            {
                this->sResponse = readBuffer;

                if (conType == "application/json") {
                    fetchStatus = this->processJSON();
                } else {
                    this->responseData.clear();
                    fetchStatus = true;
                }
            }

            curl_slist_free_all(slist);
            curl_easy_cleanup(curl); // always wipe our butt
        }
    }

    if (!fetchStatus) {
        this->sResponse.clear();
        this->responseData.clear();
    }

    return fetchStatus;
}

// This is used by curl to grab the response and put it into a var
size_t webAPI::writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *) userp)->append((char *) contents, size * nmemb);
    return size * nmemb;
}

bool webAPI::processJSON() {
    if (!(this->sResponse.empty())) // check it all out and parse
    {
        try {
            this->responseData = nlohmann::json::parse(this->sResponse);
            return true;
        } catch (std::string &e) {
            this->responseData["message"] = e;
            this->responseData["status"] = "failure";
        } catch (...) {
            this->responseData["message"] = "JSON parse error for endpoint.";
            this->responseData["status"] = "failure";
        }
    } else {
        this->responseData["message"] = "Error fetching data from remote.";
        this->responseData["status"] = "failure";
    }

    return false;
}