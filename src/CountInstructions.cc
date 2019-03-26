#include <stdint.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <algorithm>

const char *insnName(uint32_t insn)
{
    if(false)
        ;
#include "generated.insnnames.h"
    else
        return nullptr;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
        return 1;
    
    std::ifstream in(argv[1]);
    std::unordered_map<const char*, int> counts;
    
    while(in)
    {
        uint8_t a,b,c,d;
        a = in.get();
        b = in.get();
        c = in.get();
        d = in.get();
        
        uint32_t insn = (a << 24u) | (b << 16u) | (c << 8u) | d;
        
        counts[insnName(insn)]++;
    }
    
    std::vector<std::pair<int, std::string>> freqlist;
    for(auto freq : counts)
    {
        if(freq.first)
        {
            freqlist.emplace_back(freq.second, freq.first);
        }
    }
    std::sort(begin(freqlist), end(freqlist), [](auto a, auto b) { return b < a; });
    
    for(auto freq : freqlist)
    {
        std::cout << std::setw(30) << std::left << freq.second.c_str();
        std::cout << std::right << std::setw(10) << freq.first << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::setw(30) << std::left << "(unknown)";
    std::cout << std::right << std::setw(10) << counts[nullptr] << std::endl;

    return 0;
}
