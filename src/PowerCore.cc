#include <PowerCore.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>

#include "PowerCoreInlines.h"

void PowerCore::flushCache()
{
    std::cout << "flush cache\n";
    blocks.clear();
}

void PowerCore::flushCache(uint32_t addr, uint32_t sz)
{
    flushCache();
}

void PowerCore::execute()
{
    interpret2(nullptr);
    //interpret1();
}

void PowerCore::unimplemented(const char* name)
{
    uint32_t insn = load<uint32_t>(CIA);
    std::cerr << "Unimplemented instruction " << name << " at " << std::hex << CIA
                << ": " << std::hex << insn << std::endl << std::flush;
    exit(1);
}
