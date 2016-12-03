#include <net.hpp>
#include <core.hpp>
#include <enet/enet.h>

namespace tk {
    namespace net {

        void initialize() {
            tk_assert(enet_initialize() == 0, "Error initializing networking");
        }

    }
}