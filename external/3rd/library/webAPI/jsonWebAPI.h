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

#ifndef jsonWebAPI_H
#define jsonWebAPI_H

#include "json.hpp"

#ifdef WIN32
#include <curl.h>
#else

#include <unordered_map>
#include <curl/curl.h>

#endif

#include "../libLeff/libLeff.h"

namespace StellaBellum {
    enum HTTP {
        GET = 0, POST = 1
    };
    enum DTYPE {
        JSON = 0, RAW = 1
    };

    class webAPI {
    public:
        // useragent
        std::string userAgent;

        // constructor - can setup with the endpoint from the start
        webAPI(std::string endpoint, std::string userAgent = "StellaBellum webAPI");

        ~webAPI();

        // submits the request
        bool
        submit(const int &reqType = DTYPE::JSON, const int &getPost = HTTP::POST, const int &respType = DTYPE::JSON);

        // set the endpoint after object creation...or change the target if needed
        bool setEndpoint(const std::string endpoint);

        // get raw response
        std::string getRaw();

        // set a standard request string
        bool setData(std::string &data); // all or nothing

        // get a string from a given slot
        std::string getString(const std::string &slot);

        // get a vector of strings from a given slot
        std::unordered_map<int, std::string> getStringMap(const std::string &slot);

        // set json key and value for request
        template<typename T> bool addJsonData(const std::string &key, const T &value) {
            if (!key.empty() &&
                responseData.count(key) == 0) // only alow one of a given key for now, unless we support nesting later
            {
                requestData[key] = value;
                return true;
            }

            return false;
        }

        // get json response slot
        template<typename T> T getNullableValue(const std::string &slot) {
            if (!responseData.empty() && !slot.empty() && responseData.count(slot)) {
                return responseData[slot].get<T>();
            }

            return 0;
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

        // fetcher - returns raw response direct from remote
        bool fetch(const int &getPost = HTTP::POST, const int &mimeType = DTYPE::JSON);

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
