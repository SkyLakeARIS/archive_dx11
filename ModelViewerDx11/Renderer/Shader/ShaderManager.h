#pragma once
#include "../../framework.h"
#include "../Resources/RenderTypes.h"

namespace renderer
{
    struct SemiMaterial;
    class Renderer;

    class ShaderManager
    {
    public:
        ShaderManager(ID3D11Device& device, Renderer& renderer);
        ~ShaderManager();

        // TODO: improve - Renderer에서 해당 클래스로 실제 셰이더 컴파일 및 구성하는 코드 이동해야 함.

        void UpdateMaterial(eCbType type, const SemiMaterial& material);

        // 1. Renderer가 예약한 Cb는 ShaderManager가 관리하지 않고 앱 내부적으로 혹은 Renderer가 업데이트/바인드 처리
        // 2. Material만 구조가 잡히기 전까지 ShaderManager에게 바인딩 정보와 업데이트를 맡김. (수작업을 대신함)
        // MEMO: 머티리얼당 셰이더라는 가정이므로, 동일 셰이더에 다른 머티리얼은 대응이 안되는 문제는 나중에 해결해야 함.
        static void GetMaterialCbBindingDesc(eShader type, MaterialCbBinding& outBindingDesc);
        // MEMO: 텍스처를 여러 개 쓸 계획은 있지만, 아직 샘플러를 여러 개 쓸 계획은 없음(셰이더당 샘플러는 하나). Sampler Slot은 나중에 상황에 따라 확장.
        static void GetMaterialTextureBindSlots(eShader type, int8_t* const outBindingSlots);
        static void GetMaterialSamplerBindSlot(eShader type, int8_t& outBindingSlot);

    private:

        ID3D11Device* mDevice;
        Renderer* mRenderer;
    };
}

