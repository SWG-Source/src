//
// Created by Darth on 10/17/2016.
//

#ifndef webAPIHeartbeat_H
#define webAPIHeartbeat_H

#include <cstdio>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <signal.h>

#include "webAPI.h"

namespace StellaBellum {

    class webAPIHeartbeat {
    public:
        webAPIHeartbeat();

    private:
        const inline std::string get_selfpath() {
            char buff[PATH_MAX];
            ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff) - 1);
            if (len != -1) {
                buff[len] = '\0';
                return std::string(buff);
            }
            return std::string();
        }
    };
}

#endif //webAPIHeartbeat_H
