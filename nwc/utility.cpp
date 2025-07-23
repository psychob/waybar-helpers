#include "./utility.hpp"

#include <unistd.h>

void nwc::sleep(int miliseconds) {
    usleep(miliseconds * 1000);
}
