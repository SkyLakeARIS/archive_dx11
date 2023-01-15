#pragma once
#include "framework.h"
#include <vector>

class ModelImporter
{

public:
    ModelImporter();
    ~ModelImporter();

    void            LoadModelFromTextFile();
    void            LoadModelFromXFile();
    void            LoadModelAnby();

    size_t          GetVertexCount() const;
    size_t          GetIndexListCount() const;
    void            GetIndexList(unsigned int* indexList) const;
    void            GetVertexData(const size_t index, XMFLOAT3& pos, XMFLOAT2& tex, XMFLOAT3& norm) const;
    inline XMMATRIX GetTM() const;

    inline size_t   GetBodyVertexCount() const;
    inline size_t   GetBodyIndexListCount() const;
    inline void     GetBodyVertexData(const size_t index, XMFLOAT3& pos,  XMFLOAT3& norm) const;
    inline void     GetBodyIndexList(unsigned int* indexList) const;

    inline size_t   GetFaceVertexCount() const;
    inline size_t   GetFaceIndexListCount() const;
    inline void     GetFaceVertexData(const size_t index, XMFLOAT3& pos, XMFLOAT3& norm) const;
    inline void     GetFaceIndexList(unsigned int* indexList) const;

    inline size_t   GetHairVertexCount() const;
    inline size_t   GetHairIndexListCount() const;
    inline void     GetHairVertexData(const size_t index, XMFLOAT3& pos, XMFLOAT3& norm) const;
    inline void     GetHairIndexList(unsigned int* indexList) const;

private:

    void            StrSplit(const char* str, const char* delim, char** outStrToStore, const size_t& outArraySize);

public :

    /*
     * anbi TM
     */
    XMMATRIX mMatRootTM;    //  최상위

    XMMATRIX mMatSubTM; // 그다음

    XMMATRIX mMatBodyTM;
    XMMATRIX mMatFaceTM;
    XMMATRIX mMatHairTM;

private:
    /*
     *  anbi
     */
    size_t                      mBodyVertexCount;
    size_t                      mBodyIndexListCount;
    std::vector<XMFLOAT3>       mBodyPositionList;
    std::vector<XMFLOAT3>       mBodyNormalList;
    std::vector<unsigned int>   mBodyIndexList;

    size_t                      mFaceVertexCount;
    size_t                      mFaceIndexListCount;
    std::vector<XMFLOAT3>       mFacePositionList;
    std::vector<XMFLOAT3>       mFaceNormalList;
    std::vector<unsigned int>   mFaceIndexList;


    size_t                      mHairVertexCount;
    size_t                      mHairIndexListCount;
    std::vector<XMFLOAT3>       mHairPositionList;
    std::vector<XMFLOAT3>       mHairNormalList;
    std::vector<unsigned int>   mHairIndexList;

    /*
     *  bronya
     */
    size_t                      mVertexCount;
    size_t                      mIndexListCount;
    std::vector<XMFLOAT2>       mTexUVList;
    std::vector<XMFLOAT3>       mPositionList;
    std::vector<XMFLOAT3>       mNormalList;
    std::vector<unsigned int>   mIndexList;

};

inline void ModelImporter::GetHairIndexList(unsigned* indexList) const
{
    for (size_t i = 0; i < mHairIndexListCount * 3; ++i)
    {
        *(indexList + ((mBodyIndexListCount*3) + (mFaceIndexListCount*3) + i)) = mHairIndexList[i];
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
    for (size_t i = 0; i < mFaceIndexListCount * 3; ++i)
    {
        *(indexList + ((mBodyIndexListCount*3)+i)) = mFaceIndexList[i];
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
    for (size_t i = 0; i < mBodyIndexListCount * 3; ++i)
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
    return mHairIndexListCount*3;
}

inline size_t ModelImporter::GetHairVertexCount() const
{
    return mHairVertexCount;

}

inline size_t ModelImporter::GetFaceIndexListCount() const
{
    return mFaceIndexListCount * 3;

}

inline size_t ModelImporter::GetFaceVertexCount() const
{
    return mFaceVertexCount;

}

inline size_t ModelImporter::GetBodyIndexListCount() const
{
    return mBodyIndexListCount * 3;

}

inline size_t ModelImporter::GetBodyVertexCount() const
{
    return mBodyVertexCount;

}


inline XMMATRIX ModelImporter::GetTM() const
{
    //return mMatTM;
}
