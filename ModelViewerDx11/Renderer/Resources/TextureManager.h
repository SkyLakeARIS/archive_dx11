#pragma once
#include <atomic>
#include <queue>
#include "TextureData.h"

namespace renderer
{
    class TextureManager
    {
    public:
        TextureManager(ID3D11Device* device);
        ~TextureManager();

        bool InitDefaultTexture();

        void AddTexture(const int8_t* const filePath, HashID& outTexHash);
        void AddTextureDDS(const int8_t* const filePath, HashID& outTexHash);
        // MEMO: 사실상 렌더러를 위한 함수
        void AddTextureByHash(HashID hash, ID3D11ShaderResourceView* const srv);

        void RemoveTexture(HashID hash);

        ID3D11ShaderResourceView* GetTextureByHash(HashID hash);
        // MEMO: 나중에 생성할 때 같이 반환시켜 준다.
        int16_t GetTextureSerial(HashID hash);
        void GetDefaultTexture(HashID& outHash, int16_t& outSerialID);
    private:
        int16_t getSerialID();

        HRESULT createTextureResource(const WCHAR* fileName, WIC_FLAGS flag, D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, ID3D11ShaderResourceView** outShaderResourceView) const;
    public:
        // MEMO: Renderer가 예약한 텍스처
        static HashID sShadowTexHash;
        static int16_t sShadowTexSerialID;
        static HashID sGBufferColorTexHash;
        static int16_t sGBufferColorTexSerialID;
        static HashID sGBufferNormalTexHash;
        static int16_t sGBufferNormalTexSerialID;
        static HashID sGBufferPositionTexHash;
        static int16_t sGBufferPositionTexSerialID;
        static HashID sGBufferSpecularTexHash;
        static int16_t sGBufferSpecularTexSerialID;
        static HashID sGBufferAmbientTexHash;
        static int16_t sGBufferAmbientTexSerialID;
    private:
        static HashID sDefaultTexHash;
    private:
        ID3D11Device* mDevice;
        std::unordered_map<HashID, TextureData> mTextures;
        std::atomic_int16_t mSerial;
        // MEMO: 제거된 텍스처 데이터를 재활용하여 Serial이 증가하기만 하는 것을 막아야 함. 우선은 간단하게 처리.
        std::queue<int16_t> mFreeSerials;
    };

}
