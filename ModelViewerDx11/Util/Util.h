#pragma once
#include "../framework.h"

namespace util
{
    inline HashID GetDjb2Hash(const uint8_t* str)
    {
        HashID hash = 5381;
        int32_t c = 0;

        while (c = *str++) {
            // hash = hash * 33 + c
            hash = ((hash << 5) + hash) + c;
        }
        return hash;
    }

    inline HashID GetDjb2Hash(const int8_t* str)
    {
        HashID hash = 5381;
        int32_t c = 0;

        while (c = *str++) {
            // hash = hash * 33 + c
            hash = ((hash << 5) + hash) + c;
        }
        return hash;
    }
}
