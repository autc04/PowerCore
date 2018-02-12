#include <PowerCore.h>
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
