//
// Created by Darth on 10/17/2016.
//

#include "webAPIHeartbeat.h"

using namespace StellaBellum;

webAPIHeartbeat::webAPIHeartbeat() {
  webAPI handle = webAPI::webAPI(std::string(vxENCRYPT("https://login.stellabellum.net/metriccontroller/shoulderTap?type=server").decrypt()));

  handle.addJsonData("ip", "test");

  bool result = handle.submit(); // data is stored as a class member

  // do stuff
};
