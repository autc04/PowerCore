#include <PowerCore.h>
#include <iostream>
#include <cstring>

#if !defined(BIGENDIAN) & !defined(LITTLEENDIAN)

#if defined(powerpc) || defined(__ppc__)
#define BIGENDIAN
#elif defined(i386) \
     || defined(__x86_64) || defined(__x86_64__) \
     || defined (_M_X64) || defined(_M_IX86)
#define LITTLEENDIAN
#else
#error "Unknown CPU type"
#endif

#endif

#ifdef _MSC_VER
#include <stdlib.h>

inline uint16_t swap16(uint16_t v) { return _byteswap_ushort(v); }
inline uint32_t swap32(uint32_t v) { return _byteswap_ulong(v); }
#else
inline uint16_t swap16(uint16_t v) { return __builtin_bswap16(v); }
inline uint32_t swap32(uint32_t v) { return __builtin_bswap32(v); }
inline uint32_t swap64(uint32_t v) { return __builtin_bswap64(v); }
#endif

inline uint8_t swap(uint8_t v) { return v; }
inline int8_t swap(int8_t v) { return v; }
inline uint16_t swap(uint16_t v) { return swap16(v); }
inline int16_t swap(int16_t v) { return swap16(v); }
inline uint32_t swap(uint32_t v) { return swap32(v); }
inline int32_t swap(int32_t v) { return swap32(v); }
inline uint64_t swap(uint64_t v) { return swap64(v); }
inline int64_t swap(int64_t v) { return swap64(v); }


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

inline void *PowerCore::translateAddress(uint32_t addr) const
{
    return ((char*)memoryBases[addr >> 30]) + addr;
}

inline void PowerCore::setcr(int field, uint32_t x)
{
    cr = (cr & ~(0xF << (7-field)*4)) | (x << (7-field)*4);
}
inline uint32_t PowerCore::getcr(int field) const
{
    return (cr >> (7-field)*4) & 0xF;
}

inline void PowerCore::record(int field, int32_t val)
{
    if(val < 0)
        setcr(field, 8 | SO);
    else if(val > 0)
        setcr(field, 4 | SO);
    else
        setcr(field, 2 | SO);
}

inline void PowerCore::overflow(int64_t val)
{
    OV = ((val >> 63) ^ (val >> 31)) & 1;
    SO = SO || OV;
}

inline void PowerCore::crbit(int bit, bool v)
{
    cr = (cr & (0x7FFFFFFFU >> bit)) | (v << (31-bit));
}
inline bool PowerCore::crbit(int bit) const
{
    return (cr & (0x80000000U >> bit)) != 0;
}

template<typename T>
T PowerCore::load(uint32_t addr)
{
#ifdef LITTLEENDIAN
    return swap(*(T*) translateAddress(addr));
#else
    return *(T*) translateAddress(addr);
#endif
}

template<>
float PowerCore::load<float>(uint32_t addr)
{
    uint32_t i = load<uint32_t>(addr);
    float f;
    static_assert(sizeof(float) == 4, "unexpected float size");
    std::memcpy(&f, &i, 4);
    return f;
}

template<>
double PowerCore::load<double>(uint32_t addr)
{
    uint32_t i = load<uint64_t>(addr);
    double d;
    static_assert(sizeof(double) == 8, "unexpected double size");
    std::memcpy(&d, &i, 8);
    return d;
}

template<typename T>
void PowerCore::store(uint32_t addr, T value)
{
#ifdef LITTLEENDIAN
    *(T*) translateAddress(addr) = swap(value);
#else
    *(T*) translateAddress(addr) = value;
#endif
}

template<>
void PowerCore::store<float>(uint32_t addr, float value)
{
    uint32_t u;
    static_assert(sizeof(float) == 4, "unexpected float size");
    std::memcpy(&u, &value, 4);
    store(addr, u);
}

template<>
void PowerCore::store<double>(uint32_t addr, double value)
{
    uint64_t u;
    static_assert(sizeof(double) == 8, "unexpected double size");
    std::memcpy(&u, &value, 8);
    store(addr, u);
}

inline bool PowerCore::conditional(uint32_t BO, uint32_t BI)
{
    bool ctr_ok;
    if((BO & 0x4) == 0)
    {
        ctr_ok = (BO & 0x2) ? ctr == 0 : ctr != 0;
        --ctr;
    }
    else
    {
        ctr_ok = true;
    }    
    bool cond_ok;
    if(BO & 0x10)
        cond_ok = true;
    else if(BO & 0x8)
        cond_ok = (cr & (0x80000000U >> BI)) != 0;
    else
        cond_ok = (cr & (0x80000000U >> BI)) == 0;

    return ctr_ok && cond_ok;
}

//#define LOG_TRACE

void PowerCore::interpret1()
{
    uint32_t insn;
    while(CIA != 0xFFFFFFFC)
    {
#ifdef LOG_TRACE
        std::cerr << "instruction at " << std::hex << CIA
           << ": " << std::flush;
#endif
        insn = load<uint32_t>(CIA);
        
        uint32_t NIA = CIA + 4;
#ifdef LOG_TRACE
        std::cerr << std::hex << insn << std::endl;
        std::cerr << "r3: " << std::hex << r[3] << " ";
        std::cerr << "r4: " << std::hex << r[4] << " ";
        std::cerr << "r5: " << std::hex << r[5] << " ";
        std::cerr << "r6: " << std::hex << r[6] << " ";
        std::cerr << "r7: " << std::hex << r[7] << " ";
        std::cerr << "r8: " << std::hex << r[8] << " ";
        std::cerr << "r9: " << std::hex << r[9] << " ";
        std::cerr << "r10: " << std::hex << r[10] << " ";
        std::cerr << "cr: " << std::hex << cr << std::endl << std::flush;
#endif

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
                << ": " << std::hex << insn << std::endl << std::flush;
    exit(1);
}
