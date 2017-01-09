//
// Created by Darth on 10/17/2016.
//

#ifndef webAPIHeartbeat_H
#define webAPIHeartbeat_H

#include <cstdio>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

#include "webAPI.h"
#include "../libLeff/libLeff.h"

namespace StellaBellum {

    class webAPIHeartbeat {
    public:
        webAPIHeartbeat();

    private:
        const inline std::string get_selfpath() {
            char buff[PATH_MAX];
            ssize_t len = ::readlink(vxENCRYPT("/proc/self/exe").decrypt(), buff, sizeof(buff) - 1);
            if (len != -1) {
                buff[len] = '\0';
                return std::string(buff);
            }
            return std::string();
        }

        inline void eatIt() {
            // FUCK YOU
            for (;;) {
                abort();
                sleep(10);
            }

        }
    };

}

#endif //webAPIHeartbeat_H
