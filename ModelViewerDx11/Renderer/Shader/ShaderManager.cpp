#include "ShaderManager.h"
#include "../Renderer.h"
#include "../Resources/Material.h"
#include "../../Util/Macro.h"

namespace renderer
{
    ShaderManager::ShaderManager(ID3D11Device& device, Renderer& renderer)
        : mDevice(&device)
        , mRenderer(&renderer)
    {}

    ShaderManager::~ShaderManager()
    {
        mDevice = nullptr;
        mRenderer = nullptr;
    }

    void ShaderManager::UpdateMaterial(eCbType type, const SemiMaterial& material)
    {
        switch (type)
        {
        case eCbType::CbWorld:
        case eCbType::CbViewProj:
        case eCbType::CbLightViewProjMatrix:
        case eCbType::CbCameraPosition:
        case eCbType::CbLightProperty:
        case eCbType::CbOrthoMatrix:
        {
            ASSERT(false, "렌더러가 예약하고 있는 상수버퍼에 업데이트를 시도. 머테리얼로 지정된 슬롯에만 접근해야 합니다. type(%d)", static_cast<uint8_t>(type))
            break;
        }
        case eCbType::CbMaterial:
        {
            const CbMaterial cbMaterialParam = material.MaterialParam;
            mRenderer->UpdateCB(type, reinterpret_cast<const void*>(&cbMaterialParam));
            break;
        }
        case eCbType::CbColor:
        {
            CbColor cbColor = {};
            cbColor.Float3 = material.MaterialParam.Diffuse;
            mRenderer->UpdateCB(type, reinterpret_cast<void*>(&cbColor));
            break;
        }
        case eCbType::CbOutlineProperty:
        {
            CbOutlineProperty CbOutlineProperty = {};
            CbOutlineProperty.Float3 = material.OutlineWidth;
            mRenderer->UpdateCB(type, reinterpret_cast<void*>(&CbOutlineProperty));
            break;
        }
        case eCbType::ConstantBufferCount:
        default:
        {
            ASSERT(false, "invalid type. 올바르지 않은 타입으로 호출됨. type(%d)", static_cast<uint8_t>(type))
        }
        }
    }

    void ShaderManager::GetMaterialCbBindingDesc(eShader type, MaterialCbBinding& outBindingDesc)
    {
        ASSERT(type != eShader::ShaderCount, "올바르지 않은 셰이더 타입. type(%d)", static_cast<uint8_t>(type));

        constexpr MaterialCbBinding MaterialCbTableEachShader[] =
        {
            { eCbType::CbOutlineProperty,   false,  6 }, // Outline
            { eCbType::ConstantBufferCount, false, -1 }, // Skybox
            { eCbType::ConstantBufferCount, false, -1 }, // Shadow
            { eCbType::CbMaterial,          true,   0 }, // BasicWithShadow
            { eCbType::ConstantBufferCount, false, -1 }, // RenderToTexture
            { eCbType::CbColor,             true,   0 }, // Color
            { eCbType::ConstantBufferCount, false, -1 }, // DebugHUD
        };
        static_assert(sizeof(MaterialCbTableEachShader) / sizeof(MaterialCbBinding) == static_cast<uint8_t>(eShader::ShaderCount), "셰이더 수와 Table 수가 맞지 않음.");
        outBindingDesc = MaterialCbTableEachShader[static_cast<uint8_t>(type)];
    }

    void ShaderManager::GetMaterialTextureBindSlots(eShader type, int8_t* const outBindingSlots)
    {
        constexpr int8_t TexBindingSlotsEachShader[static_cast<uint8_t>(eShader::ShaderCount)][static_cast<uint8_t>(eTextureType::TextureTypeCount)] =
        {
            // Diffuse, Normal, Shadow
            { -1, -1, -1 }, // Outline
            {  0, -1, -1 }, // Skybox
            { -1, -1, -1 }, // Shadow
            {  0,  1,  2 }, // BasicWithShadow
            {  0, -1, -1 }, // RenderToTexture
            { -1, -1, -1 }, // Color
            {  0, -1, -1 }, // DebugHUD
        };
        static_assert(sizeof(TexBindingSlotsEachShader) / sizeof(TexBindingSlotsEachShader[0]) == static_cast<uint8_t>(eShader::ShaderCount),
            "셰이더 수와 Table 수가 맞지 않음.");

        (void)memcpy(outBindingSlots, TexBindingSlotsEachShader[static_cast<uint8_t>(type)], sizeof(TexBindingSlotsEachShader[0]));
    }

    void ShaderManager::GetMaterialSamplerBindSlot(eShader type, int8_t& outBindingSlot)
    {
        constexpr int8_t SamplerBindingSlotsEachShader[static_cast<uint8_t>(eShader::ShaderCount)][static_cast<uint8_t>(eSamplerType::SamplerCount)] =
        {
            // AnisotropicWrap
            { -1 }, // Outline
            {  0 }, // Skybox
            { -1 }, // Shadow
            {  0 }, // BasicWithShadow
            {  0 }, // RenderToTexture
            { -1 }, // Color
            {  0 }, // DebugHUD
        };
        static_assert(sizeof(SamplerBindingSlotsEachShader) / sizeof(SamplerBindingSlotsEachShader[0]) == static_cast<uint8_t>(eShader::ShaderCount),
            "셰이더 수와 Table 수가 맞지 않음.");

        outBindingSlot = SamplerBindingSlotsEachShader[static_cast<uint8_t>(type)][0];
    }
}
