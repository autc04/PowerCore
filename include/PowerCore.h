#include <stdint.h>
#include <unordered_map>
#include <vector>

class PowerCore
{
public:
    uint32_t r[32] = { 0 };
    double f[32] = { 0.0 };
    uint32_t cr = 0;
    uint32_t lr = 0;
    uint32_t ctr = 0;

    uint32_t CIA = 0;

    // XER
    bool SO = false;
    bool OV = false;
    bool CA = false;
    
    void *memoryBases[4] = { 0,0,0,0 };


    uint32_t getXER() const;
    void setXER(uint32_t xer);

    uint32_t (*syscall)(PowerCore&) = nullptr;

    void (*debugger)(PowerCore&) = nullptr;
    uint32_t (*getNextBreakpoint)(uint32_t addr) = nullptr;

    void execute();
    void flushCache();
    void flushCache(uint32_t from, uint32_t size);
private:
    void setcr(int field, uint32_t x);
    uint32_t getcr(int field) const;
    void record(int field, int32_t val);
    void overflow(int64_t val);
    void crbit(int bit, bool v);
    bool crbit(int bit) const;

    void frecord();

    void * translateAddress(uint32_t addr) const;

    template<typename T>
    T load(uint32_t addr);

    template<typename T>
    void store(uint32_t addr, T value);

    bool conditional(uint32_t BO, uint32_t BI);

    void unimplemented(const char* name);

    void interpret1();
    void interpret2(void ***opcodeRet);

    uint8_t *fetchBlock(uint32_t addr);

    std::unordered_map<uint32_t, std::vector<uint8_t>> blocks;
};
