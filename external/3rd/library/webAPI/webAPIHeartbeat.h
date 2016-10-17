//
// Created by Darth on 10/17/2016.
//

#ifndef webAPIHeartbeat_H
#define webAPIHeartbeat_H

#include <unordered_list>

#include "webAPI.h"
#include "../libLeff/libLeff.h"


namespace StellaBellum {

    class webAPIHeartbeat {
    public:
        webAPIHeartbeat:webAPIHeartbeat();
        webAPIHeartbeat:~webAPIHeartbeat();

        bool sendHeartbeat(std::unordered_map<std::string, std::string> metricsData = nullptr);
    };

}

#endif //webAPIHeartbeat_H
