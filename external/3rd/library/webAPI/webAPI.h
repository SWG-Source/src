/*
 * Version: 1.7
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

#include <unordered_map>
#include <openssl/ssl.h>
#include <openssl/bio.h>
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

        // our ssl cert
        inline static CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm) {
            X509_STORE *store;
            X509 *cert = NULL;
            BIO *bio;

            char * ctx = "-----BEGIN CERTIFICATE-----\n"\
				"MIIEojCCA4qgAwIBAgIUJ88p38SKi9SeyVOF0AQne1O6Vs4wDQYJKoZIhvcNAQEL\n"\
				"BQAwgYsxCzAJBgNVBAYTAlVTMRkwFwYDVQQKExBDbG91ZEZsYXJlLCBJbmMuMTQw\n"\
				"MgYDVQQLEytDbG91ZEZsYXJlIE9yaWdpbiBTU0wgQ2VydGlmaWNhdGUgQXV0aG9y\n"\
				"aXR5MRYwFAYDVQQHEw1TYW4gRnJhbmNpc2NvMRMwEQYDVQQIEwpDYWxpZm9ybmlh\n"\
				"MB4XDTE2MTIzMTA1MDcwMFoXDTMxMTIyODA1MDcwMFowYjEZMBcGA1UEChMQQ2xv\n"\
				"dWRGbGFyZSwgSW5jLjEdMBsGA1UECxMUQ2xvdWRGbGFyZSBPcmlnaW4gQ0ExJjAk\n"\
				"BgNVBAMTHUNsb3VkRmxhcmUgT3JpZ2luIENlcnRpZmljYXRlMIIBIjANBgkqhkiG\n"\
				"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwv0X8DT+AvVAWeLZvBZ+uQXFA5SEmY3w47uT\n"\
				"cwR/KCIrty7JLlswDv7iGV4f58vDAcNZq3Rs85eBY2kEatYZUBEFw+FhQDw76R9r\n"\
				"ZRj/gRfKyjkoHmmJ9ItP6YEIGHW5GGvSsB0PqV52pAESfIc4ABSUQVghLCmXCHPv\n"\
				"vMQjnTgAxgRQ0tvy52At9E39qClk+4uofMHzwk4bOKRUA9aLHLdZJQDEKR7EdZY1\n"\
				"qPIh3Rkari0aTVBf+0mnXQJ0xnIvVPc+GPYVotQ0tutISUtVPzpia0PmmbhHN4uE\n"\
				"ZVS53gOjgPz1dT/yivrsKw5i0vBqRcwMZ4dU+yfAL4uibJqwOwIDAQABo4IBJDCC\n"\
				"ASAwDgYDVR0PAQH/BAQDAgWgMBMGA1UdJQQMMAoGCCsGAQUFBwMBMAwGA1UdEwEB\n"\
				"/wQCMAAwHQYDVR0OBBYEFCjf0EvN/w5pDVLXf4dk4yfU5A02MB8GA1UdIwQYMBaA\n"\
				"FCToU1ddfDRAh6nrlNu64RZ4/CmkMEAGCCsGAQUFBwEBBDQwMjAwBggrBgEFBQcw\n"\
				"AYYkaHR0cDovL29jc3AuY2xvdWRmbGFyZS5jb20vb3JpZ2luX2NhMC8GA1UdEQQo\n"\
				"MCaCEiouc3RlbGxhYmVsbHVtLm5ldIIQc3RlbGxhYmVsbHVtLm5ldDA4BgNVHR8E\n"\
				"MTAvMC2gK6AphidodHRwOi8vY3JsLmNsb3VkZmxhcmUuY29tL29yaWdpbl9jYS5j\n"\
				"cmwwDQYJKoZIhvcNAQELBQADggEBAGXNQW26rnr4k+2hfOxkuGGMXBuYAzLcCwbg\n"\
				"H5KRH3HoJg1FmkjGC07nptDk2EAkqp6DphwTangyw0oREEIU/l2k8AvkX0WVFXdx\n"\
				"FnVWq5IenZF8dX0m9oQyH/CsF89dkvU+zksP4wzJAMvGiB8Tmc8bKWmIfBnusj3D\n"\
				"npbKvZL2ch+hwY4SZspJLoKJ4iz5wWSHihwNYxm+KGsJpt2moV15gAuObmDg7nu6\n"\
				"owOLXtbf62tQOXnXee2peBN1JX/mCHKUSL1mu+wJXjitBEgXJRGSnZl4IGv/m8Q5\n"\
				"KDeA44tJg2f/le+MertWN/+aTYhK8exu4v/7SaEJHNCwbXCJICg=\n"\
				"-----END CERTIFICATE-----\n";
			
            bio = BIO_new_mem_buf(ctx, -1);
            PEM_read_bio_X509(bio, &cert, 0, NULL);
            if (cert == NULL) {
                printf("cert is null");
		return CURLE_FAILED_INIT;
            }

            store = SSL_CTX_get_cert_store((SSL_CTX *) sslctx);

            if (X509_STORE_add_cert(store, cert) == 0) {
		printf("couldn't store cert");
                return CURLE_FAILED_INIT;
            }

            X509_free(cert);
            BIO_free(bio);

            return CURLE_OK;
        }

    protected:
        // http response code (200, 404, etc)
        long statusCode;
    };
}
#endif
