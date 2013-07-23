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

    const char * suite[2] = {
        "What's wrong with you? 别灰心! http://t.cn/zQz0Rn",
        "台北真的是天子骄子吗？",};

    for (int i = 0; i < 2; ++ i) {
        words.clear();
        int len = segmentor_segment(engine, suite[i], words);
        for (int i = 0; i < len; ++ i) {
            std::cout << words[i];
            if (i+1 == len) std::cout <<std::endl;
            else std::cout<< "|";
        }
    }
    segmentor_release_segmentor(engine);
    return 0;
}

