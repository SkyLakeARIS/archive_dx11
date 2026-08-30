#include "TextureManager.h"
#include "../../Util/Define.h"
#include "../../Util/Macro.h"
#include "../../Util/Util.h"

namespace renderer
{
    HashID TextureManager::sShadowTexHash = util::GetDjb2Hash(reinterpret_cast<const int8_t* const>("/AssetData/Generated/ShadowMap.tex"));
    int16_t TextureManager::sShadowTexSerialID = -1;
    HashID TextureManager::sDefaultTexHash = 0;
    
    TextureManager::TextureManager(ID3D11Device* device)
        : mDevice(device)
        , mSerial(0)
    {
        ASSERT(device, "invalid device. device is nullptr.");
        mTextures.reserve(64);
    }

    TextureManager::~TextureManager()
    {
        // MEMO: shadow는 Renderer가 생성했으므로 Renderer가 지움
        const auto& shadowTexIt = mTextures.find(sShadowTexHash);
        if(shadowTexIt != mTextures.end())
        {
            mTextures.erase(shadowTexIt);
        }

        for (auto& texIt: mTextures)
        {
            const uint32_t remainRef = texIt.second.SRV->Release();
            if(remainRef != 0)
            {
                ASSERT(false, "깔끔하게 정리되지 않은 텍스처 리소스 발견. 강제 삭제 처리했으나, 조치 필요. Hash(%u), Serial(%d), RemainRef(%u)", texIt.second.Hash, texIt.second.SerialID, remainRef);
                while (texIt.second.SRV->Release())
                {
                }
            }
            texIt.second.SRV = nullptr;
        }
        mTextures.clear();
        mDevice = nullptr;
    }

    bool TextureManager::InitDefaultTexture()
    {
        const int8_t* const DEFAULT_TEXTURE_FILE_PATH = reinterpret_cast <const int8_t* const>("AssetData/textures/default.png");
        const HashID defaultTextureHash = util::GetDjb2Hash(DEFAULT_TEXTURE_FILE_PATH);

        wchar_t texFilePath[util::MAX_PATH_LENGTH];
        const uint32_t filePathLength = strlen(reinterpret_cast<char const*>(DEFAULT_TEXTURE_FILE_PATH));
        size_t  numConverted = 0;
        (void)mbstowcs_s(&numConverted, texFilePath, util::MAX_PATH_LENGTH, reinterpret_cast<char const*>(DEFAULT_TEXTURE_FILE_PATH), filePathLength);
        ASSERT(numConverted == (filePathLength + 1), "conversion result is incorrect. converted(%u) pathLength(%u)", numConverted, filePathLength);

        D3D11_SHADER_RESOURCE_VIEW_DESC texDefaultDesc = {};
        texDefaultDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        texDefaultDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        texDefaultDesc.Texture2D.MipLevels = 1;
        texDefaultDesc.Texture2D.MostDetailedMip = 0;

        ID3D11ShaderResourceView* srv = nullptr;
        if (FAILED(createTextureResource(texFilePath, WIC_FLAGS_NONE, texDefaultDesc, &srv)))
        {
            ASSERT(false, "FAIL : CreateTextureResource - default texture");
            return false;
        }
        SET_PRIVATE_DATA(srv, "DefaultTexture");

        AddTextureByHash(defaultTextureHash, srv);
        sDefaultTexHash = defaultTextureHash;
        return true;
    }

    void TextureManager::AddTexture(const int8_t* const filePath, HashID& outTexHash)
    {
        ASSERT(filePath != nullptr, "invalid path. (filePath is nullptr) ");
        ASSERT(*filePath != ' ', "invalid path. (filePath may empty string) ");
        ASSERT(*filePath != '\0', "invalid path. (filePath may empty(null) string) ");

        outTexHash = 0;
        const HashID texHash = util::GetDjb2Hash(filePath);
        const auto& texIt = mTextures.find(texHash);
        if(texIt == mTextures.end())
        {
            wchar_t texFilePath[util::MAX_PATH_LENGTH];
            const uint32_t filePathLength = strlen(reinterpret_cast<char const*>(filePath));
            size_t  numConverted = 0;
            (void)mbstowcs_s(&numConverted, texFilePath, util::MAX_PATH_LENGTH, reinterpret_cast<char const*>(filePath), filePathLength);
            ASSERT(numConverted == (filePathLength + 1), "conversion result is incorrect. converted(%u) pathLength(%u)", numConverted, filePathLength);

            ScratchImage rawImage;
            if(FAILED(LoadFromWICFile(texFilePath, WIC_FLAGS_NONE, nullptr, rawImage)))
            {
                return;
            }

            ID3D11Resource* tex = nullptr;
            if (FAILED(CreateTexture(mDevice, rawImage.GetImages(), rawImage.GetImageCount(), rawImage.GetMetadata(), &tex)))
            {
                rawImage.Release();
                return;
            }

            ID3D11ShaderResourceView* srv = nullptr;
            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipLevels = 1;
            desc.Texture2D.MostDetailedMip = 0;
            if (FAILED(mDevice->CreateShaderResourceView(tex, &desc, &srv)))
            {
                ASSERT(false, "ShaderResourceView 생성 실패");
                SAFETY_RELEASE(tex);
                rawImage.Release();
                return;
            }
            SAFETY_RELEASE(tex);
            rawImage.Release();

            TextureData newTexData = {};
            newTexData.Hash = texHash;
            newTexData.SRV = srv;
            newTexData.SerialID = getSerialID();
            mTextures.insert(std::make_pair(texHash, newTexData));
            outTexHash = texHash;
        }
        else
        {
            texIt->second.SRV->AddRef();
            outTexHash = texHash;
        }
    }

    void TextureManager::AddTextureDDS(const int8_t* const filePath, HashID& outTexHash)
    {
        outTexHash = 0;
        const HashID texHash = util::GetDjb2Hash(filePath);
        const auto& texIt = mTextures.find(texHash);
        if (texIt == mTextures.end())
        {
            wchar_t texFilePath[util::MAX_PATH_LENGTH];
            const uint32_t filePathLength = strlen(reinterpret_cast<char const*>(filePath));
            size_t  numConverted = 0;
            (void)mbstowcs_s(&numConverted, texFilePath, util::MAX_PATH_LENGTH, reinterpret_cast<char const*>(filePath), filePathLength);
            ASSERT(numConverted == (filePathLength + 1), "conversion result is incorrect. converted(%u) pathLength(%u)", numConverted, filePathLength);

            ScratchImage image;
            if (FAILED(LoadFromDDSFile(texFilePath, DDS_FLAGS_NONE, nullptr, image)))
            {
                ASSERT(false, "failed to load DDS file : DDS 로드 실패");
                return;
            }

            ID3D11Texture2D* textureResource = nullptr;
            if (FAILED(CreateTexture(mDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), (ID3D11Resource**)&textureResource)))
            {
                image.Release();
                ASSERT(false, "failed to create TextureResource : gTextureResource 생성 실패");
                return;
            }
            D3D11_TEXTURE2D_DESC desc;
            textureResource->GetDesc(&desc);

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MipLevels = 1;
            srvDesc.TextureCube.MostDetailedMip = 0;
            srvDesc.Format = desc.Format;
            srvDesc.TextureCube.MipLevels = desc.MipLevels;

            ID3D11ShaderResourceView* srv = nullptr;
            if (FAILED(mDevice->CreateShaderResourceView(textureResource, &srvDesc, &srv)))
            {
                ASSERT(false, "failed to create outShaderResourceView : outShaderResourceView 생성 실패");
                image.Release();
                SAFETY_RELEASE(textureResource);
                return;
            }

            TextureData newTexData = {};
            newTexData.Hash = texHash;
            newTexData.SRV = srv;
            newTexData.SerialID = getSerialID();
            mTextures.insert(std::make_pair(texHash, newTexData));
            outTexHash = texHash;

            image.Release();
            SAFETY_RELEASE(textureResource);
        }
        else
        {
            texIt->second.SRV->AddRef();
            outTexHash = texHash;
        }
    }

    void TextureManager::AddTextureByHash(HashID hash, ID3D11ShaderResourceView* const srv)
    {
        ASSERT(srv, "유효하지 않은 srv.(%p)", srv);
        ASSERT(hash, "유효하지 않은 Hash (%u)", hash);
        const auto& texIt = mTextures.find(hash);
        if (texIt == mTextures.end())
        {
            TextureData newTexData = {};
            newTexData.Hash = hash;
            newTexData.SRV = srv;
            newTexData.SerialID = getSerialID();
            mTextures.insert(std::make_pair(hash, newTexData));
        }
        else
        {
            texIt->second.SRV->AddRef();
        }
    }

    void TextureManager::RemoveTexture(HashID hash)
    {
        ASSERT(hash > 0, "Hash is invalid. may not initialized. Hash(%d)", hash);
        const auto& texIt = mTextures.find(hash);
        if(texIt != mTextures.end())
        {
            // MEMO:SRV의 RefCount를 사용하는 것은 어떨지라는 아이디어로 사용.
            const uint32_t remainRef = texIt->second.SRV->Release();
            if(remainRef == 0)
            {
                mFreeSerials.push(texIt->second.SerialID);
                texIt->second.SerialID = 0;
                mTextures.erase(texIt);
            }
        }
    }

    ID3D11ShaderResourceView* TextureManager::GetTextureByHash(HashID hash)
    {
        ASSERT(hash > 0, "Hash is invalid. Hash(%d)", hash);
        const auto& texIt = mTextures.find(hash);
        if(texIt == mTextures.end())
        {
            return nullptr;
        }
        return texIt->second.SRV;
    }

    int16_t TextureManager::GetTextureSerial(HashID hash)
    {
        int16_t serial = -1;
        ASSERT(hash > 0, "Hash is invalid. Hash(%d)", hash);
        const auto& texIt = mTextures.find(hash);
        if (texIt != mTextures.end())
        {
            serial = texIt->second.SerialID;
        }
        return serial;
    }

    void TextureManager::GetDefaultTexture(HashID& outHash, int16_t& outSerialID)
    {
        ASSERT(sDefaultTexHash != 0, "기본 텍스처가 등록되지 않음");
        const auto& texIt = mTextures.find(sDefaultTexHash);
        if (texIt != mTextures.end())
        {
            outHash = texIt->second.Hash;
            outSerialID = texIt->second.SerialID;
        }
    }

    int16_t TextureManager::getSerialID()
    {
        int16_t newSerialID;
        if(mFreeSerials.empty())
        {
            newSerialID = mSerial.load(std::memory_order_relaxed);
            mSerial.fetch_add(1, std::memory_order_relaxed);
            ASSERT(newSerialID < INT16_MAX, "serialKey 한계치.");
        }
        else
        {
            newSerialID = mFreeSerials.front();
            mFreeSerials.pop();
        }

        return newSerialID;
    }


    HRESULT TextureManager::createTextureResource(const WCHAR* fileName, WIC_FLAGS flag, D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, ID3D11ShaderResourceView** outShaderResourceView) const
    {
        ScratchImage image;
        ID3D11Texture2D* textureResource = nullptr;

        HRESULT result = LoadFromWICFile(fileName, flag, nullptr, image);
        if (FAILED(result))
        {
            ASSERT(false, "failed to load imamge file : 이미지 로드 실패");
            goto FAILED;
        }

        result = CreateTexture(mDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), (ID3D11Resource**)(&textureResource));
        if (FAILED(result))
        {
            ASSERT(false, "failed to create TextureResource : gTextureResource 생성 실패");
            goto FAILED;
        }
        D3D11_TEXTURE2D_DESC desc;
        textureResource->GetDesc(&desc);
        srvDesc.Format = desc.Format;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        result = mDevice->CreateShaderResourceView((ID3D11Resource*)textureResource, &srvDesc, &(*outShaderResourceView));
        if (FAILED(result))
        {
            ASSERT(false, "failed to create outShaderResourceView : outShaderResourceView 생성 실패");
            goto FAILED;
        }

        result = S_OK;
    FAILED:
        image.Release();
        SAFETY_RELEASE(textureResource);
        return result;
    }

}
