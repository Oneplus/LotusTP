#include <iostream>
#include <string>
#include "segment_dll.h"

int main(int argc, char * argv[]) {
    if (argc < 2) {
        std::cerr << "cws [model path]" << std::endl;
        return 1;
    }

    void * engine = segmentor_create_segmentor(argv[1]);
    if (!engine) {
        return -1;
    }
    std::vector<std::string> words;
    int len = segmentor_segment(engine, 
            "爱上一匹野马，可我的家里没有草原。", words);
    for (int i = 0; i < len; ++ i) {
        std::cout << words[i] << "|";
    }
    std::cout << std::endl;
    segmentor_release_segmentor(engine);
    return 0;
}