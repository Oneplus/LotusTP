#include <iostream>
#include "cfgparser.hpp"
#include "logging.hpp"
#include "postagger.h"

using namespace ltp::utility;
using namespace ltp::postagger;

void usage(void) {
    std::cerr << "otpos - Training and testing suite for Part of Speech Tagging" << std::endl;
    std::cerr << "Copyright (C) 2012-2013 HIT-SCIR" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: ./otpos <config_file>" << std::endl;
    std::cerr << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        return -1;
    }

    ConfigParser cfg(argv[1]);

    if (!cfg) {
        ERROR_LOG("Failed to parse config file.");
        return -1;
    }

    Postagger engine(cfg);
    engine.run();
    return 0;
}
