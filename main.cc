#include "PowerCore.h"

#include <fstream>
#include <vector>
#include <stdint.h>
#include <cassert>
#include <iostream>

void PowerCore::interpret1()
{
    uint32_t insn;
    while(CIA != 0xFFFFFFFC)
    {
        //std::cerr << "instruction at " << std::hex << CIA
         //   << ": " << std::flush;
        insn = load<uint32_t>(CIA);
        
        uint32_t NIA = CIA + 4;
        //std::cerr << std::hex << insn << std::endl;
        //std::cout << "r3: " << std::dec << r[3] << std::endl;
        //std::cout << "cr: " << std::hex << cr << std::endl;

        if(false)
            ;
#include "generated.interpret1.h"
        else
            unimplemented("(unknown)");

        CIA = NIA;
    }
}

void PowerCore::unimplemented(const char* name)
{
    uint32_t insn = load<uint32_t>(CIA);
    std::cerr << "Unimplemented instruction " << name << " at " << std::hex << CIA
                << ": " << std::hex << insn << std::endl;
}


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
