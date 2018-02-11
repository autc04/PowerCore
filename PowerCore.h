#include <stdint.h>

class PowerCore
{
public:
    uint32_t    r[32];
    //uint32_t    cr[8];
    //bool        crso[8];
    uint32_t cr;

    // XER
    bool SO;
    bool OV;
    bool CA;
    

private:
    void setcr(int field, uint32_t x);
    void record(int field, int32_t val);
    void overflow(int64_t val);
    void crbit(int bit, bool v);
    bool crbit(int bit);

};

inline void PowerCore::setcr(int field, uint32_t x)
{
    cr = (cr & (~0xF >> 4*field)) | x;
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
inline bool PowerCore::crbit(int bit)
{
    return (cr & (0x80000000U >> bit)) != 0;
}
