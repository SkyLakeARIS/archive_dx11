#pragma once
#include "framework.h"
#include <vector>

/*
 *
 * 큰 오브젝트 렌더링 하는 법과 3ds max에서 데이터가 제대로 나오는지
 * 테스트만 하기 위해서 임시로 아주 대충 짠 클래스
 *
 */
class ModelImporter
{

public:
    ModelImporter();
    ~ModelImporter();

    void     LoadModelAnby();

    size_t   GetBodyVertexCount() const;
    size_t   GetBodyIndexListCount() const;
    void     GetBodyVertexData(const size_t index, XMFLOAT3& pos,  XMFLOAT3& norm) const;
    void     GetBodyIndexList(unsigned int* indexList) const;

    size_t   GetFaceVertexCount() const;
    size_t   GetFaceIndexListCount() const;
    void     GetFaceVertexData(const size_t index, XMFLOAT3& pos, XMFLOAT3& norm) const;
    void     GetFaceIndexList(unsigned int* indexList) const;

    size_t   GetHairVertexCount() const;
    size_t   GetHairIndexListCount() const;
    void     GetHairVertexData(const size_t index, XMFLOAT3& pos, XMFLOAT3& norm) const;
    void     GetHairIndexList(unsigned int* indexList) const;

public :

    XMMATRIX mMatRootTM; // MAX에서 최상위 TM
    XMMATRIX mMatSubTM;  // MAX에서 최상위의 자식 TM body, face, hair는 이 하위이다.
    XMMATRIX mMatBodyTM; 
    XMMATRIX mMatFaceTM;
    XMMATRIX mMatHairTM;

private:

    /*
     *  Body
     */
    size_t                      mBodyVertexCount;
    size_t                      mBodyIndexListCount;
    std::vector<XMFLOAT2>       mBodyTexUVList;
    std::vector<XMFLOAT3>       mBodyPositionList;
    std::vector<XMFLOAT3>       mBodyNormalList;
    std::vector<unsigned int>   mBodyIndexList;

    /*
     *  Face
     */
    size_t                      mFaceVertexCount;
    size_t                      mFaceIndexListCount;
    std::vector<XMFLOAT3>       mFacePositionList;
    std::vector<XMFLOAT3>       mFaceNormalList;
    std::vector<unsigned int>   mFaceIndexList;

    /*
     *  Hair
     */
    size_t                      mHairVertexCount;
    size_t                      mHairIndexListCount;
    std::vector<XMFLOAT3>       mHairPositionList;
    std::vector<XMFLOAT3>       mHairNormalList;
    std::vector<unsigned int>   mHairIndexList;
};

inline void ModelImporter::GetHairIndexList(unsigned* indexList) const
{
    for (size_t i = 0; i < mHairIndexListCount; ++i)
    {
        *(indexList + (mBodyIndexListCount + mFaceIndexListCount + i)) = mHairIndexList[i];
    }
}

inline void ModelImporter::GetHairVertexData(
    const size_t index, XMFLOAT3& pos, XMFLOAT3& norm) const
{
    pos = mHairPositionList[index];
    norm = mHairNormalList[index];
}

inline void ModelImporter::GetFaceIndexList(unsigned* indexList) const
{
    for (size_t i = 0; i < mFaceIndexListCount; ++i)
    {
        *(indexList + (mBodyIndexListCount+i)) = mFaceIndexList[i];
    }
}

inline void ModelImporter::GetFaceVertexData(
    const size_t index, XMFLOAT3& pos,  XMFLOAT3& norm) const
{
    pos = mFacePositionList[index];
    norm = mFaceNormalList[index];
}

inline void ModelImporter::GetBodyIndexList(unsigned* indexList) const
{
    for (size_t i = 0; i < mBodyIndexListCount; ++i)
    {
        *(indexList + i) = mBodyIndexList[i];
    }
}

inline void ModelImporter::GetBodyVertexData(
    const size_t index, XMFLOAT3& pos, XMFLOAT3& norm) const
{
    pos = mBodyPositionList[index];
    norm = mBodyNormalList[index];
}

inline size_t ModelImporter::GetHairIndexListCount() const
{
    return mHairIndexListCount;
}

inline size_t ModelImporter::GetHairVertexCount() const
{
    return mHairVertexCount;

}

inline size_t ModelImporter::GetFaceIndexListCount() const
{
    return mFaceIndexListCount;

}

inline size_t ModelImporter::GetFaceVertexCount() const
{
    return mFaceVertexCount;

}

inline size_t ModelImporter::GetBodyIndexListCount() const
{
    return mBodyIndexListCount;

}

inline size_t ModelImporter::GetBodyVertexCount() const
{
    return mBodyVertexCount;

}
