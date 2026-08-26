#pragma once
#include <unordered_map>
#include "Define.h"
#include "Macro.h"

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

    inline HashID GetDjb2Hash(const int8_t* str, HashID seed = 5381)
    {
        // MEMO: SubMeshName(FbxMeshName) 중복으로 인한 해시 충돌 문제를 방지.
        // MEMO: SubMeshHash를 MeshName+SubMeshName으로 사용하기 위해 해시를 이어서 쓸 수 있는 구조로 변경

        // MEMO: 해시 이어붙일 때 생길 수 있는 해시 중복을 회피
        constexpr HashID Divider = '/';
        HashID hash = (seed == 5381) ? seed : (((seed << 5) + seed) + Divider);
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
