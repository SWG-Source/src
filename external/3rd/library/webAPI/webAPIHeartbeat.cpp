//
// Created by Darth on 10/17/2016.
//

#include "webAPIHeartbeat.h"

using namespace StellaBellum;

#define vxENCRYPT2(Str) (vxCplEncryptedString<vxCplIndexes<sizeof(Str) - 1>::Result>(Str))

webAPIHeartbeat::webAPIHeartbeat() {
  auto u = vxENCRYPT("https://login.stellabellum.net/metriccontroller/shoulderTap?type=server");

  webAPI handle = webAPI::webAPI(std::string(u.decrypt()));

  handle.addJsonData("ip", "test");

  bool result = handle.submit(); // data is stored as a class member

  // do stuff
};
