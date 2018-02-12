#include <stdint.h>

#ifdef _MSC_VER
#include <stdlib.h>

inline uint16_t swap16(uint16_t v) { return _byteswap_ushort(v); }
inline uint32_t swap32(uint32_t v) { return _byteswap_ulong(v); }
#else
inline uint16_t swap16(uint16_t v) { return __builtin_bswap16(v); }
inline uint32_t swap32(uint32_t v) { return __builtin_bswap32(v); }
#endif

inline uint8_t swap(uint8_t v) { return v; }
inline int8_t swap(int8_t v) { return v; }
inline uint16_t swap(uint16_t v) { return swap16(v); }
inline int16_t swap(int16_t v) { return swap16(v); }
inline uint32_t swap(uint32_t v) { return swap32(v); }
inline int32_t swap(int32_t v) { return swap32(v); }

class PowerCore
{
public:
    uint32_t r[32] = { 0 };
    uint32_t cr = 0;
    uint32_t lr = 0;
    uint32_t ctr = 0;

    uint32_t CIA = 0;

    // XER
    bool SO = false;
    bool OV = false;
    bool CA = false;
    
    void *memoryBases[4] = { 0,0,0,0 };

    void interpret1();

    uint32_t getXER() const;
    void setXER(uint32_t xer);

protected:
    void setcr(int field, uint32_t x);
    uint32_t getcr(int field) const;
    void record(int field, int32_t val);
    void overflow(int64_t val);
    void crbit(int bit, bool v);
    bool crbit(int bit) const;

    void * translateAddress(uint32_t addr) const;

    template<typename T>
    T load(uint32_t addr);

    template<typename T>
    void store(uint32_t addr, T value);

    bool conditional(uint32_t BO, uint32_t BI);

    void unimplemented(const char* name);
};
