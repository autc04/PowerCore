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

void PowerCore::setXER(uint32_t XER)
{
    SO = !! (XER & (1<<31));
    OV = !! (XER & (1<<30));
    CA = !! (XER & (1<<29));
}

uint32_t PowerCore::getXER() const
{
    return ((uint32_t)SO << 31) | ((uint32_t)OV << 30) | ((uint32_t)CA << 29);
}
