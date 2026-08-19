#pragma once
#include "../framework.h"
#include "Define.h"

namespace util
{
#if defined(_DEBUG)
    inline void CheckHashCollision(HashID hash, const int8_t* const str)
    {
        struct HashSource
        {
            int8_t SourceString[util::MAX_NAME_LENGTH];
        };
        static std::unordered_map<HashID, HashSource> sStorage;

        const auto& sourceIt = sStorage.find(hash);
        if (sourceIt == sStorage.end())
        {
            HashSource source;
            const int16_t orgStrLength = static_cast<int16_t>(strlen(reinterpret_cast<char const*>(str)));
            ASSERT(orgStrLength + 1 < util::MAX_NAME_LENGTH, "str이 버퍼 사이즈보다 큼. 입력 문자열을 검점하거나, util의 Length 상수 조정 필요");
            (void)memcpy(source.SourceString, str, orgStrLength + 1);
            sStorage.insert(std::make_pair(hash, std::move(source)));
        }
        else
        {
            // MEMO: 다른게 두개 이상 들어가면 충돌이므로 하나만 체크하면 됨.
            // MEMO: 갯수 셀바에 그냥 비교로
            if (strcmp(reinterpret_cast<char const*>(sourceIt->second.SourceString), reinterpret_cast<char const*>(str)) != 0)
            {
                ASSERT(false, "해시 충돌 감지 \n시도: %hs \n대상: %hs\n", str, sourceIt->second.SourceString);
            }
        }
    }
#endif

    inline HashID GetDjb2Hash(const int8_t* str)
    {
        HashID hash = 5381;
        int32_t c = 0;
#if defined(_DEBUG)
        const int8_t* const sourceStr = str;
#endif

        while (c = *str++) {
            // hash = hash * 33 + c
            hash = ((hash << 5) + hash) + c;
        }
#if defined(_DEBUG)
        CheckHashCollision(hash, sourceStr);
#endif

        return hash;
    }
}
