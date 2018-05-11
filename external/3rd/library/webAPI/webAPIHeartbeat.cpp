//
// Created by Darth on 10/17/2016.
//

#include "webAPIHeartbeat.h"

using namespace StellaBellum;

webAPIHeartbeat::webAPIHeartbeat() {
    webAPI api(std::string("https://login.stellabellum.net/metric/shoulderTap"), std::string("StellaBellum WebAPI Metrics Sender"));
    api.addJsonData<std::string>(std::string("type"), std::string("server"));

    api.submit();
}
