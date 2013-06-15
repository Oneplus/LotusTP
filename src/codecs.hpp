/*
 * This code is modified from @file utf.h in @project zpar<
 * 
 *  @author: ZHANG, Yue <http://www.sutd.edu.sg/yuezhang.aspx>
 *  @modifier: LIU, Yijia <yjliu@ir.hit.edu.cn>
 */
#ifndef __CODECS_HPP__
#define __CODECS_HPP__

#include <iostream>
#include <vector>

namespace ltp {
namespace strutils {
namespace codecs {
enum { UTF8, GBK };

using namespace std;
int decode(const std::string & s, 
        std::vector<std::string>& chars, int encoding=UTF8) {
    unsigned long int idx=0;
    unsigned long int len=0;
    chars.clear();

    if (encoding == UTF8) {
        while (idx<s.length()) {
            if ((s[idx]&0x80)==0) {
                chars.push_back(s.substr(idx, 1));
                ++len;
                ++idx;
            } else if ((s[idx]&0xE0)==0xC0) {
                chars.push_back(s.substr(idx, 2));
                ++len;
                idx+=2;
            } else if ((s[idx]&0xF0)==0xE0) {
                chars.push_back(s.substr(idx, 3));
                ++len;
                idx+=3;
            } else {
                std::cerr << "Warning: " 
                    << "in utf.h "
                    << "getCharactersFromUTF8String: string '" 
                    << s 
                    << "' not encoded in unicode utf-8" 
                    << std::endl;
                 ++len;
                 ++idx;
            }
        }
    } else if (encoding == GBK) {
        while (idx<s.length()) {
            if ((s[idx]&0x80)==0) {
                chars.push_back(s.substr(idx, 1));
                ++ len;
                ++ idx;
            } else {
                chars.push_back(s.substr(idx, 2));
                ++ len;
                idx += 2;
            }
        }
    } else {
        return 0;
    }
}

}       //  end for namespace codecs
}       //  end for namespace strutils
}       //  end for namespace ltp
#endif  //  end for __CODECS_HPP__
