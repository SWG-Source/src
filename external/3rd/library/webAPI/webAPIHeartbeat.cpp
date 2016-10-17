//
// Created by Darth on 10/17/2016.
//

#include "webAPIHeartbeat.h"

using namespace StellaBellum;

webAPIHeartbeat:webAPIHeartbeat() {
  vxCplEncryptedString u = vxENCRYPT("https://login.stellabellum.net/metriccontroller/shoulderTap?type=server");

  webAPI handle = webAPI::webAPI(u.decrypt());

  handle.addJsonData("ip", "test");

  bool result = fetch.submit(); // data is stored as a class member

  // do stuff

  delete handle;
};