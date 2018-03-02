#include <PowerCore.h>
#include "PowerCoreInlines.h"
#include <iostream>
#include <iomanip>

//#define LOG_TRACE

[[maybe_unused]] static void disass1(uint32_t insn)
{
    if(false)
        ;
#include "generated.disass.h"
    else
        std::clog << std::left << std::setfill(' ') << std::setw(10) << "dc" << std::hex << insn << std::endl;
}

void PowerCore::interpret1()
{
    uint32_t insn;
    uint32_t breakpoint = 0xFFFFFFFF;

    if(getNextBreakpoint)
        breakpoint = getNextBreakpoint(CIA);
    while(CIA != 0xFFFFFFFC)
    {
        if(CIA == breakpoint)
        {
            if(debugger)
                debugger(*this);
            if(getNextBreakpoint)
                breakpoint = getNextBreakpoint(CIA+4);
        }
#ifdef LOG_TRACE
        std::clog.clear();
        std::clog << "instruction at " << std::hex << CIA
           << ": " << std::flush;
#endif
        insn = load<uint32_t>(CIA);
        
        uint32_t NIA = CIA + 4;
#ifdef LOG_TRACE
        std::clog << std::hex << insn << std::endl;
        std::clog << "    ";
        disass1(insn);

        std::clog << "r3: " << std::hex << r[3] << " ";
        std::clog << "r4: " << std::hex << r[4] << " ";
        std::clog << "r5: " << std::hex << r[5] << " ";
        std::clog << "r6: " << std::hex << r[6] << " ";
        std::clog << "r7: " << std::hex << r[7] << " ";
        std::clog << "r8: " << std::hex << r[8] << " ";
        std::clog << "r9: " << std::hex << r[9] << " ";
        std::clog << "r10: " << std::hex << r[10] << " ";
        std::clog << "cr: " << std::hex << cr << std::endl << std::flush;
#endif

        if(false)
            ;
#include "generated.interpret1.h"
        else
            unimplemented("(unknown)");

        if(getNextBreakpoint && NIA != CIA + 4)
            breakpoint = getNextBreakpoint(NIA);

        if(NIA != CIA + 4)
            std::clog << "jump: " << std::hex << NIA << " (from " << CIA << ")" << std::endl;
        CIA = NIA;
    }
}

using TranslatedOpcode = void*;
#include "generated.opcodes.h"


uint8_t *PowerCore::fetchBlock(uint32_t blockAddr)
{
    auto it = blocks.find(blockAddr);
    if(it != blocks.end())
    {
        //std::cout << "Again at " << std::hex << blockAddr << std::endl;
        return it->second.data();
    }
    std::vector<uint8_t> block;

    std::cout << "First time at " << std::hex << blockAddr << std::endl;

    auto allocOpcode = [&](unsigned sz) {
        block.resize(block.size() + sz);
        return block.data() + block.size() - sz;
    };

    void ** opcodes;
    interpret2(&opcodes);
    auto makeOpcode = [opcodes](unsigned idx) { return opcodes[idx]; };

    uint32_t addr = blockAddr;
    for(;;)
    {
        uint32_t insn = load<uint32_t>(addr);
        if(false)
            ;
#include "generated.translate.h"
        else
            unimplemented("unknown during translation");
        addr += 4;
    }

    return blocks.emplace(blockAddr, std::move(block)).first->second.data();
}

void PowerCore::interpret2(void ***opcodesRet)
{
    if(opcodesRet)
    {
        static void *opcodes[] = {
#include "generated.opcodelabels.h"
        };
        *opcodesRet = &opcodes[0];
        return;
    }

loop:
    if(CIA == 0xFFFFFFFC)
    {
        std::cout << "Exit address reached.\n";
        return;
    }
    if(CIA == 0)
    {
        std::cout << "NULL\n";
        debugger(*this);
    }
    uint8_t *code = fetchBlock(CIA);
    {
        //if(debugger)
        //    debugger(*this);

        goto *(*(TranslatedOpcode*)code);

#include "generated.interpret2.h"
    }
    std::cout << "Fallthrough.\n";
    std::abort();
}
