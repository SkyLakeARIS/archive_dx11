#pragma once
#include "framework.h"
#include <vector>

class ModelImporter
{
    struct VertexInfo
    {
        XMFLOAT3 pos;
        XMFLOAT3 norm;
        XMFLOAT2 tex;
    };

    struct ObjectNode
    {
        FbxNode* parent;
        FbxNode* current;
    };
public:
    ModelImporter();
    ~ModelImporter();

    void            Release();

    void            LoadFbxModel();

    inline size_t   GetFbxVertexCount() const;
    inline size_t   GetFbxIndexListCount() const;
    inline void     GetFbxIndexList(unsigned int* indexList) const;
    inline void     GetFbxVertexData(const size_t index, XMFLOAT3& pos, XMFLOAT2& tex, XMFLOAT3& norm) const;

private:

    void        printNode(FbxNode* node);
    FbxString   getAttributeTypeName(FbxNodeAttribute::EType type);
    void        printAttribute(FbxNodeAttribute* pAttribute);

    void        preprocess(FbxNode* parent, FbxNode* current);

    void        parseMesh();

private:

    FbxManager* mFbxManager;
    FbxScene*   mFbxScene;
    /*
     *  fbx data
     */
    std::vector<VertexInfo>     mFbxVertexInfo;
    std::vector<unsigned int>   mFbxIndexList;
    std::vector<ObjectNode>     mFbxObjects;

};

inline void ModelImporter::GetFbxVertexData(
    const size_t index, XMFLOAT3& pos, XMFLOAT2& tex, XMFLOAT3& norm) const
{
    pos = mFbxVertexInfo[index].pos;
    norm = mFbxVertexInfo[index].norm;
    tex = mFbxVertexInfo[index].tex;
}

inline void ModelImporter::GetFbxIndexList(unsigned* indexList) const
{
    for (size_t i = 0; i < mFbxIndexList.size(); ++i)
    {
        *(indexList + i) = mFbxIndexList[i];
    }
}

inline size_t ModelImporter::GetFbxIndexListCount() const
{
    return mFbxIndexList.size();
}

inline size_t ModelImporter::GetFbxVertexCount() const
{
    return mFbxVertexInfo.size();
}
