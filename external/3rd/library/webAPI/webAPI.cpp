/*
 * Version: 1.6
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

CURLcode* webAPI::sslctx_function(CURL *curl, void *sslctx, void *parm) {
    X509_STORE *store;
    X509 *cert = NULL;
    BIO *bio;
    char *mypem = /* this is the cloudflare self signed for stellabellum.net, good for 30 years */
            "-----BEGIN CERTIFICATE-----\n"\
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

    bio = BIO_new_mem_buf(mypem, -1);
    PEM_read_bio_X509(bio, &cert, 0, NULL);
    if (cert == NULL) {
        printf("PEM_read_bio_X509 failed...\n");
    }

    store = SSL_CTX_get_cert_store((SSL_CTX *) sslctx);

    if (X509_STORE_add_cert(store, cert) == 0) {
        printf("error adding certificate\n");
    }

    X509_free(cert);
    BIO_free(bio);

    return CURLE_OK;
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

std::unordered_map<int, std::string> webAPI::getStringMap(const std::string &slot) {
    std::unordered_map<int, std::string> ret = std::unordered_map<int, std::string>();

    if (!this->responseData.empty() && !slot.empty() && responseData.count(slot) &&
        !this->responseData[slot].is_null()) {

        nlohmann::json j = this->responseData[slot];

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

            curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback); // place the data into readBuffer using writeCallback
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // specify readBuffer as the container for data
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

            if (this->uri.find("login.stellabellum.net") != std::string::npos) {
                curl_easy_setopt(ch, CURLOPT_SSLCERTTYPE, "PEM");
                curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 1L);
                curl_easy_setopt(ch, CURLOPT_SSL_CTX_FUNCTION, this->sslctx_function);
            }

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
