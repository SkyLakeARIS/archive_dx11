#include "Light.h"
#include "Camera.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/BufferManager.h"
#include "../Renderer/Resources/RenderPacket.h"
#include "../Util/Util.h"

namespace scene
{
    Light::Light(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 color, Camera* camera, float nearPlane, float farPlane, renderer::ShaderManager& shaderManager)
        : mPosition(pos)
        , mDirection(dir)
        , mColor(color)
        , mMatProj(XMMatrixIdentity())
        , mMatViewProj(XMMatrixIdentity())
        , mNearPlane(nearPlane)
        , mFarPlane(farPlane)
        , mCamera(camera)
    {

        mLines.reserve(24 * eCascadeLevel::Level_4);
        mCascadePlaneDistances[0] = nearPlane; // 0.1
        mCascadePlaneDistances[1] = farPlane / 100.0f;   // 5
        mCascadePlaneDistances[2] = farPlane / 50.0f; // 10 /
        mCascadePlaneDistances[3] = farPlane / 25.0f; // 20
        mCascadePlaneDistances[4] = farPlane / 10.0f; // 50
        mCascadePlaneDistances[5] = farPlane; // 500

        updateLightPropertyCB(shaderManager);
        updateMatrices(shaderManager);
    }

    Light::~Light()
    {
    }

    void Light::DrawDebug(std::vector<renderer::RenderPacket>& commandList)
    {
        for (const auto& subMesh : mMeshDebug.SubMeshes)
        {
            renderer::RenderPacket command = renderer::RenderPacket::MakeCommand(
                mMeshDebug.VertexFormat,
                renderer::GetVertexStrideSize(mMeshDebug.VertexFormat),
                renderer::eBufferUsage::Dynamic,
                false,
                subMesh.VertexRange,
                subMesh.IndexRange,
                subMesh.Material,
                renderer::eRenderTarget::Default,
                XMMatrixIdentity(),
                renderer::eShader::Color,
                renderer::eRasterType::Basic,
                renderer::eSamplerType::SamplerCount,
                0,
                renderer::ePrimitiveTopology::Lines,
                false,
                false,
                false
            );

            commandList.push_back(command);
        }
    }

    void Light::SetupCascade(renderer::Renderer& renderer, renderer::ShaderManager& shaderManager)
    {
        mLines.clear();
        for (uint32_t i = 0; i < eCascadeLevel::Level_4 - 1; ++i)
        {
            getPointsFromMatrix(&(mCamera->GetViewMatrix()), mCascadePlaneDistances[i], mCascadePlaneDistances[i + 1], &mMatLightViews[i], &mMatLightProjs[i], renderer, shaderManager);
        }
        // int32_t index = 0;
       //  mMatViewProj = mMatLightViews[index] * mMatLightProjs[index];

        //if (!mLinesBuffer)
        //{
        //    // debug - frustum
        //    D3D11_BUFFER_DESC bufferDesc = {  };
        //    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        //    bufferDesc.ByteWidth = sizeof(XMFLOAT3) * mLines.size();
        //    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        //    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        //    D3D11_SUBRESOURCE_DATA data;
        //    data.pSysMem = mLines.data();

        //    ID3D11Device* device = renderer::Renderer::GetInstance()->GetDevice();
        //    device->CreateBuffer(&bufferDesc, &data, &mLinesBuffer);
        //}
        //else
        {
            //ID3D11DeviceContext* deviceContext = renderer::Renderer::GetInstance()->GetDeviceContext();
            //D3D11_MAPPED_SUBRESOURCE subresource = {  };
            //subresource.pData = mLines.data();
            //subresource.RowPitch = sizeof(XMFLOAT3) * mLines.size();
            //deviceContext->Map(mLinesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
            ////  D3D11_MAP_FLAG::D3D11_MAP_FLAG_DO_NOT_WAIT

            //deviceContext->Unmap(mLinesBuffer, 0);
        }

        updateMatrices(shaderManager);
    }

    XMFLOAT4 Light::GetDirection() const
    {
        return XMFLOAT4(mDirection.x, mDirection.y, mDirection.z, 1.0f);
    }

    XMFLOAT3 Light::GetPosition() const
    {
        return XMFLOAT3(mPosition.x, mPosition.y, mPosition.z);
    }

    XMFLOAT4 Light::GetColor() const
    {
        return XMFLOAT4(mColor.x, mColor.y, mColor.z, 1.0f);
    }

    const XMMATRIX* const Light::GetViewProjMatrix() const
    {
        return &mMatViewProj;
    }

    void Light::updateMatrices(renderer::ShaderManager& shaderManager)
    {
        mMatView = XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mDirection), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
        //mMatProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0, 69.0f, 120.0f);
        mMatProj = XMMatrixOrthographicOffCenterLH(-1.0f, 1.0f, -1.0f, 1.0f, 69.0f, 100.0f);
        mMatViewProj = mMatView * mMatProj;

        // cascade test
        int32_t index = 0;
        mMatViewProj = mMatLightViews[index] * mMatLightProjs[index];

        renderer::CbLightViewProjMatrix cbLightVpMat;
        cbLightVpMat.Matrix = XMMatrixTranspose(mMatViewProj);
        //deviceContext->UpdateSubresource(mCB, 0, nullptr, &cbWvp, 0U, 0U);
        shaderManager.UpdateCB(renderer::eCbType::CbLightViewProjMatrix, &cbLightVpMat);
    }

    void Light::updateLightPropertyCB(renderer::ShaderManager& shaderManager)
    {
        renderer::CbLightProperty cbLightProperty;
        cbLightProperty.First = XMFLOAT4(mColor.x, mColor.y, mColor.z, 0.0f);
        cbLightProperty.Second = XMFLOAT4(mPosition.x, mPosition.y, mPosition.z, 0.0f);
        shaderManager.UpdateCB(renderer::eCbType::CbLightProperty, &cbLightProperty);
    }

    void Light::getPointsFromMatrix(XMMATRIX* matView, float nearPlane, float farPlane, XMMATRIX* const outMatLightView, XMMATRIX* const outMatLightProj, renderer::Renderer& renderer, renderer::ShaderManager& shaderManager)
    {
        XMMATRIX matLightProj = XMMatrixPerspectiveFovLH(mCamera->GetFov(), mCamera->GetAspectRatio(), nearPlane, farPlane);
        matLightProj = mCamera->GetViewMatrix() * matLightProj;
        XMMATRIX matViewProjInv = XMMatrixInverse(nullptr, matLightProj);

        // near - leftTop, rightTop, leftBottom, rightBottom
        // far - leftTop, rightTop, leftBottom, rightBottom
        XMFLOAT3 points[8] = {
            //{-1.0, 1.0, 0.0}, {1.0, 1.0, 0.0}, {-1.0, -1.0, 0.0}, {1.0, -1.0, 0.0},
            //{-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0}
            {-1.0, 1.0, 0.0}, {1.0, 1.0, 0.0}, {-1.0, -1.0, 0.0}, {1.0, -1.0, 0.0},
            {-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0}
        };

        XMFLOAT3 pointsInWorld[8] = {};
        XMVECTOR mid = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

        for (uint32_t i = 0; i < 8; ++i)
        {
            XMVECTOR point = XMLoadFloat3(&points[i]);
            point = XMVector3Transform(point, matViewProjInv);
            point /= point.m128_f32[3];
            XMStoreFloat3(&pointsInWorld[i], point);
            mid += point;
        }
        mid /= 8;

        XMVECTOR radius = XMVector3Length((XMLoadFloat3(&pointsInWorld[0]) - XMLoadFloat3(&pointsInWorld[6])));
        radius *= 0.5f;


        float texelPerUnit = 2048.0f / (radius.m128_f32[0] * 2.0f); // 2048 is shadow texture resolution
        const XMMATRIX matScale = XMMatrixScaling(texelPerUnit, texelPerUnit, texelPerUnit);

        XMVECTOR vecLightDir = XMVector3Normalize(XMLoadFloat3(&mDirection));
        XMMATRIX tempMatView = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), -vecLightDir, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        tempMatView = tempMatView * matScale;
        const XMMATRIX tempMatViewInv = XMMatrixInverse(nullptr, tempMatView);

        // 텍셀 사이즈에서(?) 프러스텀 중앙의 위치 이동
        mid = XMVector3Transform(mid, tempMatView);
        mid.m128_f32[0] = floorf(mid.m128_f32[0]);
        mid.m128_f32[1] = floorf(mid.m128_f32[1]);
        mid = XMVector3Transform(mid, tempMatViewInv);

        XMVECTOR eyePosition = mid + (vecLightDir * radius * 2.0f);

        *outMatLightView = XMMatrixLookAtLH(eyePosition, mid, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        // *outMatLightView = XMMatrixLookToLH(eyePosition, -vecLightDir, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

        float radiusF = radius.m128_f32[0];
        *outMatLightProj = XMMatrixOrthographicOffCenterLH(-radiusF, radiusF, -radiusF, radiusF, -radiusF * 6.0f, radiusF * 6.0f);


        XMFLOAT3 asdasd;
        XMStoreFloat3(&asdasd, eyePosition);
        mLines.push_back(asdasd);
        XMStoreFloat3(&asdasd, mid);
        mLines.push_back(asdasd);

        // draw frustum
        //mLines.push_back(XMFLOAT3(pointsFinal[0])); //  near
        //mLines.push_back(XMFLOAT3(pointsFinal[1]));

        //mLines.push_back(XMFLOAT3(pointsFinal[1]));
        //mLines.push_back(XMFLOAT3(pointsFinal[3]));

        //mLines.push_back(XMFLOAT3(pointsFinal[3]));
        //mLines.push_back(XMFLOAT3(pointsFinal[2]));

        //mLines.push_back(XMFLOAT3(pointsFinal[2]));
        //mLines.push_back(XMFLOAT3(pointsFinal[0]));

        //mLines.push_back(XMFLOAT3(pointsFinal[0])); // near to far
        //mLines.push_back(XMFLOAT3(pointsFinal[4]));

        //mLines.push_back(XMFLOAT3(pointsFinal[1]));
        //mLines.push_back(XMFLOAT3(pointsFinal[5]));

        //mLines.push_back(XMFLOAT3(pointsFinal[2]));
        //mLines.push_back(XMFLOAT3(pointsFinal[6]));

        //mLines.push_back(XMFLOAT3(pointsFinal[3]));
        //mLines.push_back(XMFLOAT3(pointsFinal[7]));

        //mLines.push_back(XMFLOAT3(pointsFinal[4])); // far
        //mLines.push_back(XMFLOAT3(pointsFinal[5]));

        //mLines.push_back(XMFLOAT3(pointsFinal[5]));
        //mLines.push_back(XMFLOAT3(pointsFinal[7]));

        //mLines.push_back(XMFLOAT3(pointsFinal[7]));
        //mLines.push_back(XMFLOAT3(pointsFinal[6]));

        //mLines.push_back(XMFLOAT3(pointsFinal[6]));
        //mLines.push_back(XMFLOAT3(pointsFinal[4]));


        XMFLOAT3 pointToWorld[8] = {
        };

        XMMATRIX lightViewProjInv = XMMatrixInverse(nullptr, (*outMatLightView) * (*outMatLightProj));
        XMVECTOR vecPointToWorld;
        // move points light view space to world space
        for (uint32_t i = 0; i < 8; ++i)
        {
            vecPointToWorld = XMLoadFloat3(&points[i]);
            vecPointToWorld = XMVector3Transform(vecPointToWorld, lightViewProjInv);
            vecPointToWorld /= vecPointToWorld.m128_f32[3];
            XMStoreFloat3(&pointToWorld[i], vecPointToWorld);
        }

        mLines.push_back(XMFLOAT3(pointToWorld[0])); //  near
        mLines.push_back(XMFLOAT3(pointToWorld[1]));

        mLines.push_back(XMFLOAT3(pointToWorld[1]));
        mLines.push_back(XMFLOAT3(pointToWorld[3]));

        mLines.push_back(XMFLOAT3(pointToWorld[3]));
        mLines.push_back(XMFLOAT3(pointToWorld[2]));

        mLines.push_back(XMFLOAT3(pointToWorld[2]));
        mLines.push_back(XMFLOAT3(pointToWorld[0]));

        mLines.push_back(XMFLOAT3(pointToWorld[0])); // near to far
        mLines.push_back(XMFLOAT3(pointToWorld[4]));

        mLines.push_back(XMFLOAT3(pointToWorld[1]));
        mLines.push_back(XMFLOAT3(pointToWorld[5]));

        mLines.push_back(XMFLOAT3(pointToWorld[2]));
        mLines.push_back(XMFLOAT3(pointToWorld[6]));

        mLines.push_back(XMFLOAT3(pointToWorld[3]));
        mLines.push_back(XMFLOAT3(pointToWorld[7]));

        mLines.push_back(XMFLOAT3(pointToWorld[4])); // far
        mLines.push_back(XMFLOAT3(pointToWorld[5]));

        mLines.push_back(XMFLOAT3(pointToWorld[5]));
        mLines.push_back(XMFLOAT3(pointToWorld[7]));

        mLines.push_back(XMFLOAT3(pointToWorld[7]));
        mLines.push_back(XMFLOAT3(pointToWorld[6]));

        mLines.push_back(XMFLOAT3(pointToWorld[6]));
        mLines.push_back(XMFLOAT3(pointToWorld[4]));

        if(mMeshDebug.MeshHash == 0)
        {
            int8_t virtualFilePath[util::MAX_PATH_LENGTH] = {};
            const int16_t wroteCount = sprintf_s(reinterpret_cast<char*>(virtualFilePath), util::MAX_PATH_LENGTH, "%sPrimitive_Light_Debug_Line.mesh",
                reinterpret_cast<const char*>(renderer::MeshGenerator::VIRTUAL_ROOT_PATH));

            (void)memcpy(mMeshDebug.MeshName, virtualFilePath, wroteCount + 1);

            mMeshDebug.MeshHash = util::GetDjb2Hash(virtualFilePath);
            mMeshDebug.VertexFormat = renderer::eVertexFormat::P;
        }

        // TODO: Dynamic Mesh는 SubMesh와 Material을 어떻게 처리하는 게 좋을지? - 우선은 바로 확장하지 않고 현재 구조 기준으로 수작업.
        if (mMeshDebug.SubMeshes.empty())
        {
            // no Sampler, Blend
            renderer::SubMesh newSubMesh = {};

            // TODO: 단일 메시의 경우 서브메시와 해시를 같게하는게 맞을지? .subMesh로 구분을 하는게 나을지? - 어떻게 처리하는 게 더 나을지 자료 조사하기
            (void)memcpy(newSubMesh.SubMeshName, mMeshDebug.MeshName, util::MAX_NAME_LENGTH);
            newSubMesh.SubMeshHash = mMeshDebug.MeshHash;
            newSubMesh.Material.Factors.Diffuse = XMFLOAT3(1.0f, 1.0f, 0.0f);

            mMeshDebug.SubMeshes.push_back(std::move(newSubMesh));
        }

        renderer::SubMesh& subMesh= mMeshDebug.SubMeshes.front();

        const int16_t strideVertex = renderer::GetVertexStrideSize(mMeshDebug.VertexFormat);
        renderer::BufferManager* const bufferManager = renderer.GetBufferManager();
        bufferManager->AddVertexDynamic(reinterpret_cast<int8_t*>(mLines.data()), strideVertex * mLines.size(), subMesh.SubMeshHash, strideVertex, subMesh.VertexRange);

        mMeshDebug.VertexRange = subMesh.VertexRange;
    }
}
