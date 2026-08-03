#pragma once
#include <queue>

#include "TextureData.h"
#include "../../framework.h"

namespace renderer
{
    class TextureManager
    {
    public:
        TextureManager(ID3D11Device* device);
        ~TextureManager();

        void AddTexture(const int8_t* const filePath, HashID& outTexHash);
        void AddTextureDDS(const int8_t* const filePath, HashID& outTexHash);

        void RemoveTexture(HashID hash);

        ID3D11ShaderResourceView* GetTextureByHash(HashID hash);
        // MEMO: 나중에 생성할 때 같이 반환시켜 준다.
        int16_t GetTextureSerial(HashID hash);
    private:
        int16_t getSerialID();
    private:
        ID3D11Device* mDevice;
        std::unordered_map<HashID, TextureData> mTextures;
        std::atomic_int16_t mSerial;
        // MEMO: 제거된 텍스처 데이터를 재활용하여 Serial이 증가하기만 하는 것을 막아야 함. 우선은 간단하게 처리.
        std::queue<int16_t> mFreeSerials;
    };

}
