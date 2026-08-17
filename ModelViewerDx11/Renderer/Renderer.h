#pragma once
#include "../framework.h"
#include "./Resources/RenderTypes.h"

namespace renderer
{
    class ShaderManager;
    struct RenderPacket;
}

namespace renderer
{
    class BufferManager;
    class TextureManager;

    class Renderer final : IUnknown
    {
    private:
        struct RenderTargetDepthStencilMap
        {
            uint32_t RenderTargetIndex;
            uint32_t NumViews;
            uint32_t DepthStencilIndex;
        };
        typedef RenderTargetDepthStencilMap RtvDsMap;

        struct PrimitiveTopologyMap
        {
            ePrimitiveTopology UserType;
            D3D11_PRIMITIVE_TOPOLOGY ApiType;
        };

    public:
        Renderer();
        ~Renderer();


        void SetManagers(BufferManager* const bufferManager, TextureManager* const textureManager, ShaderManager* const shaderManager);

        // D3D
        HRESULT CreateDeviceAndSetup(DXGI_SWAP_CHAIN_DESC& swapChainDesc, uint32 width, uint32 height, bool bDebugMode);
        HRESULT CreateRenderTargetView(ID3D11Texture2D* const texture, D3D11_RENDER_TARGET_VIEW_DESC* const desc, ID3D11RenderTargetView** outRtv, const char* const debugTag = "NO_INFO") const;
        HRESULT CreateDepthStencilView(ID3D11Texture2D* const texture, D3D11_DEPTH_STENCIL_VIEW_DESC* const desc, ID3D11DepthStencilView** outDs, const char* const debugTag = "NO_INFO") const;

        // 그림자 매핑을 위한 설계
        void    SetViewport(bool bFullScreen) const;
        HRESULT CreateShadowRenderTarget();

        // init - program
        bool initialize(HWND handleWindow, int16_t width, int16_t height, int16_t frameRate);

        // TODO: API 의존성을 완전히 분리하려면 desc 조차도 분리하는 게 좋을 것 같다. 일단은 이대로 사용
        HRESULT CreateBlendState(D3D11_BLEND_DESC& desc, HashID& outHash);
        // Cate : texture 
        HRESULT CreateTexture2D(D3D11_TEXTURE2D_DESC& desc, ID3D11Texture2D** outTex, const char* tag) const;

        // Renderer 
        void ClearScreenAndDepth(eRenderTarget type) const;
        void ClearDepthBuffer() const;
        void Present() const;

        void    Cleanup();
        bool CheckDeviceLost(bool& outIsReInitialize) const;


        // COM
        ULONG   AddRef() override;
        ULONG   Release() override;
        HRESULT QueryInterface(const IID& riid, void** ppvObject) override;

        //  D3D state

        void BindCbToVsByType(uint32_t slot, uint32_t numBuffer, eCbType type) const;
        void BindCbToPs(uint32_t slot, uint32_t numBuffer, eCbType type) const;

        void BindVertexBuffer(uint32_t stride) const;
        void BindIndexBuffer() const;
        void BindVertexBufferDynamic(uint32_t stride) const;
        void BindIndexBufferDynamic() const;

        void BindSamplerToPsByType(uint32_t slot, eSamplerType type) const;
        void BindBlendStateByHash(HashID hash, const float* const blendFactors, uint32_t mask);
        void BindTextureToPs(uint32_t slot, HashID textureHash) const;
        // TODO: improve - eTextureType과 충돌이 없으면서 preset을 쓸 방법을 나중에 고민해 보자(default/shadow). 우선은 texture분리를 위해 이렇게
        void BindShadowTextureToPs(uint32_t slot) const;
        void BindDefaultTextureToPs(uint32_t slot) const;
        void BindRasterStateByType(eRasterType type) const;
        void BindDepthStencilState(bool bSkybox) const; // 현재는 스카이박스만 사용하므로

        void UnbindTexturePs(uint32_t slot) const;

        void BindPrimitiveTopologyTo(D3D_PRIMITIVE_TOPOLOGY topology) const;
        void BindPrimitiveTopologyByType(ePrimitiveTopology topology) const;
        void BindRenderTargetTo(eRenderTarget type);
        void BindInputLayoutTo(eVertexFormat type) const;
        void BindShaderTo(eShader type) const;

        // Draw
        void Draw(uint32_t vertexCount, uint32_t startVertexLocation) const;
        void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) const;

        // getter
        ID3D11Device*           GetDevice() const;
        ID3D11DeviceContext*    GetDeviceContext() const;

        void GetCurrentPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY& outTopology) const;
        BufferManager* const GetBufferManager() const;
    public:
        // MEMO: BlendState의 다양한 옵션을 대응하기 위해 비트 슬라이싱을 통해 해시 계산
        static inline HashID GetBlendStateHash(D3D11_BLEND_DESC& desc);
        // Debug
        static void CheckLiveObjects();
    private:
        // MEMO: textureManager가 초기화된 후, 렌더러가 사용하는 텍스처를 추가. 등록되면 Manager가 수명 관리
        void registerShadowTexture();


        bool    createRasterState();
        HRESULT createSamplerState();

    private:


        ULONG                       mRefCount;

        // D3D Device
        ID3D11Device*               mDevice;
        ID3D11DeviceContext*        mDeviceContext;

        // 
        IDXGISwapChain*             mSwapChain;

        ID3D11Texture2D*            mDepthStencilTexture;
        ID3D11DepthStencilState*    mSkyboxDepthStencil;

        // render target, depthStencil
        // 일단은 쉽게 무조건 1:1매핑으로 (nullptr 처리는 나중에 최적화)
        ID3D11RenderTargetView* mRenderTargetViewList[static_cast<uint8_t>(eRenderTarget::RenderTargetCount)];
        ID3D11DepthStencilView* mDepthStencilViewList[static_cast<uint8_t>(eRenderTarget::RenderTargetCount)];
        RtvDsMap mRtvDsMapTable[static_cast<uint8_t>(eRenderTarget::RenderTargetCount)]; // combine rtv - depth-stencil pairs

        // shadow
        ID3D11Texture2D*           mTexShadow;
        ID3D11Texture2D*           mTexColor;
        ID3D11ShaderResourceView*  mShadowSrv;
        ID3D11ShaderResourceView** mCascadeShadowSrvList;
        D3D11_VIEWPORT             mViewportFull;
        D3D11_VIEWPORT             mViewportTex;

        // raster state
        ID3D11RasterizerState*      mRasterStates[static_cast<uint32>(eRasterType::RasterCount)]; // 0: back cull, 1: front cull

        // sampler state
        ID3D11SamplerState* mSamplerState[static_cast<uint8_t>(eSamplerType::SamplerCount)];
        // blend state
        // MEMO: option이 많고, 블렌드 하는데 조합이 많을 것 같으니 Hash로 관리하는 게 나을 것 같다.
        std::unordered_map<HashID, ID3D11BlendState*> mBlendStateMap;
        // topology
        PrimitiveTopologyMap mPrimitiveTopologies[static_cast<uint8_t>(ePrimitiveTopology::TopologyCount)];
        // Managers
        BufferManager* mBufferManager;
        TextureManager* mTextureManager;
        ShaderManager* mShaderManager;
    };
}
