#include "PowerCore.h"

#include <fstream>
#include <vector>
#include <stdint.h>
#include <cassert>
#include <iostream>


int main()
{
    std::ifstream in("testfun.bin");
    in.seekg(0, std::ios::end);
    size_t sz = in.tellg();
    in.seekg(0);

    std::vector<char> code(sz);
    in.read(code.data(),sz);

    std::vector<char> stack(1024 * 1024);

    PowerCore core;

    core.memory_bases[0] = code.data();
    core.memory_bases[1] = stack.data() - 0x40000000;
    core.r[1] = 0x40000000 + 1024 * 1024 - 1024;

    for(int i = 0; i < 15; i++)
    {
        core.r[3] = i;
        std::cout << "in: " << std::dec << core.r[3] << std::endl;
        
        core.CIA = 0;
        core.lr = 0xFFFFFFFC;
        core.interpret1();

        std::cout << "out: " << std::dec << core.r[3] << std::endl;
    }

    return 0;
}
