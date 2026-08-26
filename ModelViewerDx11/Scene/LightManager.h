#pragma once
#include "../Util/Type.h"

namespace scene
{
    class Light;

    class LightManager final
    {
    private:
        static const uint32_t MAX_LIGHT_NUM = 16;

        struct LightContainer
        {
            LightContainer(Light* const light, uint32_t id)
                : Light(light)
                , Id(id)
            {
            }
            Light* Light;
            uint32_t Id;
        };
    public:

        static LightManager* GetInstance();
        static void             Release();

        void AddLight(Light* const light, uint32_t& outId);

        void FindLight(uint32_t id, Light* outLight);
        void RemoveLight(uint32_t id);
    private:

        LightManager();
        ~LightManager();
    private:
        static LightManager* mInstance;
        static uint32_t       mIdCount;

        LightContainer* mLights[MAX_LIGHT_NUM];
        uint32_t mLightCount;

        static Light* GlobalLight;
    };
}
