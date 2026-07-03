#include "paradigm.h"
#include <iostream>

#include "cosmology.h"
#include "pico/stdlib.h"
#include "quetzal.h"

using namespace quetzal;

int main() {
    stdio_init_all();
    std::cout << "(~) quetzal init..." << std::endl;

    Quetzal quetzal{};
    std::cout << "(~) quetzal init..." << std::endl;

#ifdef DMX
    quetzal.dmx_loop();
#elifdef KEYHOLE
    quetzal.keyhole_loop();
#elifdef SIGURD
    quetzal.sigurd_loop();
#endif
}
