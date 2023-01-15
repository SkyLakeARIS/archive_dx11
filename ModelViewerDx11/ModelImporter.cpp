#include "ModelImporter.h"
#include <fstream>
#include <string>

ModelImporter::ModelImporter()
    : mVertexCount(0)
    , mIndexListCount(0)
{
    mPositionList.reserve(32768);
    mNormalList.reserve(32768);
    mTexUVList.reserve(32768);
    mIndexList.reserve(80000);
}

ModelImporter::~ModelImporter()
{
    
}

void ModelImporter::LoadModelFromTextFile()
{
    std::ifstream stream;
    stream.open(L"C:\\Users\\bunke\\바탕 화면\\ModelData\\test_modelVertex.txt");
    if(!stream.is_open())
    {
        ASSERT(false, "test_modelVertex 파일 열기에 실패했습니다.");
        return;
    }

    /*
     *  vertex 
     */
    std::string line;
    line.reserve(64);

    std::getline(stream, line);
    line.clear();
    std::getline(stream, line);

    sscanf_s((char*)line.c_str(), "vertex count : %ud ", &mVertexCount);
    line.clear();

    if(mVertexCount <= 0)
    {
        ASSERT(false, "버텍스의 수가 0입니다.");
        return;
    }
    //

    XMFLOAT3 pos;
    for(int count = 0; count < mVertexCount; ++count)
    {
        std::getline(stream, line);
        sscanf_s((char*)line.c_str(), "%f, %f, %f", &pos.x, &pos.y, &pos.z);
        
        mPositionList.push_back(XMFLOAT3(pos));

        ZeroMemory(&pos, sizeof(pos));
        line.clear();
    }

    stream.close();


    /*
     * normal
     */
    stream.open(L"C:\\Users\\bunke\\바탕 화면\\ModelData\\test_modelNormal.txt");
    if (!stream.is_open())
    {
        ASSERT(false, "test_modelNormal 파일 열기에 실패했습니다.");
        return;
    }

    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    for (int count = 0; count < mVertexCount; ++count)
    {
        std::getline(stream, line);
        sscanf_s((char*)line.c_str(), "%f, %f, %f", &pos.x, &pos.y, &pos.z);

        mNormalList.push_back(XMFLOAT3(pos));

        ZeroMemory(&pos, sizeof(pos));
        line.clear();
    }

    stream.close();

    /*
     *  Tex UV
     */
    stream.open(L"C:\\Users\\bunke\\바탕 화면\\ModelData\\test_modelTexUV.txt");
    if (!stream.is_open())
    {
        ASSERT(false, "test_modelTexUV 파일 열기에 실패했습니다.");
        return;
    }

    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    for (int count = 0; count < mVertexCount; ++count)
    {
        std::getline(stream, line);
        sscanf_s((char*)line.c_str(), "%f, %f", &pos.x, &pos.y);

        mTexUVList.push_back(XMFLOAT2(pos.x, pos.y));

        ZeroMemory(&pos, sizeof(pos));
        line.clear();
    }

    stream.close();

    /*
     *  Index List
     */
    stream.open(L"C:\\Users\\bunke\\바탕 화면\\ModelData\\test_modelIndex.txt");
    if (!stream.is_open())
    {
        ASSERT(false, "test_modelIndex 파일 열기에 실패했습니다.");
        return;
    }

    std::getline(stream, line);
    line.clear();
    std::getline(stream, line);
    sscanf_s((char*)line.c_str(), "face count : %ud ", &mIndexListCount);

    line.clear();

    unsigned int triangle[3] = { 0, };
    for (int count = 0; count < mIndexListCount; ++count)
    {
        std::getline(stream, line);
        sscanf_s((char*)line.c_str(), "%d.0, %d.0, %d.0", &triangle[0], &triangle[1], &triangle[2]);

        mIndexList.push_back(triangle[0]);
        mIndexList.push_back(triangle[1]);
        mIndexList.push_back(triangle[2]);

       // ZeroMemory(&triangle, sizeof(triangle));
        line.clear();
    }

    stream.close();

    /*
     *  TM
     */
    stream.open(L"C:\\Users\\bunke\\바탕 화면\\ModelData\\test_modelTM.txt");
    if (!stream.is_open())
    {
        ASSERT(false, "test_modelTM 파일 열기에 실패했습니다.");
        return;
    }

    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    float mat[16] = { 0.0f, };
    std::getline(stream, line);
    sscanf_s((char*)line.c_str(), "%f, %f, %f, %f:%f, %f, %f, %f:%f, %f, %f, %f:%f, %f, %f, %f",
        &mat[0], &mat[1], &mat[2], &mat[3], 
        &mat[4], &mat[5], &mat[6], &mat[7], 
        &mat[8], &mat[9], &mat[10], &mat[11], 
        &mat[12], &mat[13], &mat[14], &mat[15]);

    //mMatTM = XMMATRIX(mat);

    line.clear();

    stream.close();

}

void ModelImporter::LoadModelFromXFile()
{
    std::ifstream stream;

    stream.open("C:\\Users\\bunke\\바탕 화면\\ModelData\\test_model.x");
    if(!stream.is_open())
    {
        ASSERT(false, "test_model 파일 열기 실패");
        return;
    }

    //  최상위;;
    mMatRootTM = XMMATRIX(
        39.370080,0.000000,0.000000,0.000000,
        0.000000,-0.000002,-39.370080,0.000000,
        0.000000,39.370080,-0.000002,0.000000,
        0.000000,0.000000,0.000000,1.000000);

    // 그다음
    mMatSubTM = XMMATRIX(
        1.000000,0.000000,0.000000,0.000000,
        0.000000,1.000000,0.000000,0.000000,
        0.000000,-0.000000,1.000000,0.000000,
        0.000000,0.000000,0.000000,1.000000);

    mMatBodyTM = XMMATRIX(
        1.000000,0.000000,0.000000,0.000000,
        0.000000,-0.000000,1.000000,0.000000,
        0.000000,-1.000000,-0.000000,0.000000,
        0.000000,0.000000,0.000000,1.000000);

    mMatFaceTM = XMMATRIX(
        1.000000,0.000000,0.000000,0.000000,
        0.000000,-0.000000,1.000000,0.000000,
        0.000000,-1.000000,-0.000000,0.000000,
        0.000000,0.000000,0.000000,1.000000);

    mMatHairTM = XMMATRIX(
        1.000000,0.000000,0.000000,0.000000,
        0.000000,-0.000000,1.000000,0.000000,
        0.000000,-1.000000,-0.000000,0.000000,
        0.000000,0.000000,0.000000,1.000000);


    std::string line;
    line.reserve(64);
    std::getline(stream, line);
    line.clear();

    std::getline(stream, line);
    sscanf_s((char*)line.c_str(), "  %d;", &mVertexCount);
    line.clear();

    /*
     * vertex position
     */
    char ignore;
    float pos[3] = { 0.0f, };
    for(size_t i = 0; i < mVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "  %f;%f;%f;%c", &pos[0], &pos[1], &pos[2], &ignore);
        mPositionList.emplace_back(pos[0], pos[1], pos[2]);
        line.clear();
    }

    /*
     *  index list
     */
    std::getline(stream, line);
    sscanf_s(line.c_str(), "  %d;", &mIndexListCount);
    line.clear();

    unsigned int triangle[3] = { 0, };
    for (size_t i = 0; i < mIndexListCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "  3;%d,%d,%d;%c", &triangle[0], &triangle[1], &triangle[2], &ignore);
        mIndexList.emplace_back(triangle[0]);
        mIndexList.emplace_back(triangle[1]);
        mIndexList.emplace_back(triangle[2]);
        line.clear();
    }

    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    /*
     *  normal
     */
    for (size_t i = 0; i < mVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "   %f;%f;%f;%c", &pos[0], &pos[1], &pos[2], &ignore);
        mNormalList.emplace_back(pos[0], pos[1], pos[2]);
        line.clear();
    }

    /*
     *  index list // ignore
     */
    std::getline(stream, line);
    line.clear();
    for (size_t i = 0; i < mIndexListCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "   3;%d;%d;%d;%c", &triangle[0], &triangle[1], &triangle[2], &ignore);
        line.clear();
    }







    /*
     *  Tex uv
     */
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    for (size_t i = 0; i < mVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "   %f;%f;%c", &pos[0], &pos[1], &ignore);
        mTexUVList.emplace_back(pos[0], pos[1]);
        line.clear();
    }

    stream.close();
}

void ModelImporter::LoadModelAnby()
{
    std::ifstream stream;

    stream.open("C:\\Users\\bunke\\바탕 화면\\ModelData\\test_model2.x");
    if (!stream.is_open())
    {
        ASSERT(false, "test_model2 파일 열기 실패");
        return;
    }


    //  최상위;;
    mMatRootTM = XMMATRIX(
        39.370080, 0.000000, 0.000000, 0.000000,
        0.000000, -0.000002, -39.370080, 0.000000,
        0.000000, 39.370080, -0.000002, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000);

    // 그다음
    mMatSubTM = XMMATRIX(
        1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 1.000000, 0.000000, 0.000000,
        0.000000, -0.000000, 1.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000);

    mMatBodyTM = XMMATRIX(
        1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, -0.000000, 1.000000, 0.000000,
        0.000000, -1.000000, -0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000);

    mMatFaceTM = XMMATRIX(
        1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, -0.000000, 1.000000, 0.000000,
        0.000000, -1.000000, -0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000);

    mMatHairTM = XMMATRIX(
        1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, -0.000000, 1.000000, 0.000000,
        0.000000, -1.000000, -0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000);


    std::string line;
    line.reserve(64);
    std::getline(stream, line);
    line.clear();

    std::getline(stream, line);
    sscanf_s((char*)line.c_str(), "    %d;", &mBodyVertexCount);
    line.clear();

    /*
     * vertex position
     */
    char ignore;
    float pos[3] = { 0.0f, };
    for (size_t i = 0; i < mBodyVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "    %f;%f;%f;%c", &pos[0], &pos[1], &pos[2], &ignore);
        mBodyPositionList.emplace_back(pos[0], pos[1], pos[2]);
        line.clear();
    }

    /*
     *  index list
     */
    std::getline(stream, line);
    sscanf_s(line.c_str(), "    %d;", &mBodyIndexListCount);
    line.clear();

    unsigned int triangle[3] = { 0, };
    for (size_t i = 0; i < mBodyIndexListCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "    3;%d,%d,%d;%c", &triangle[0], &triangle[1], &triangle[2], &ignore);
        mBodyIndexList.emplace_back(triangle[0]);
        mBodyIndexList.emplace_back(triangle[1]);
        mBodyIndexList.emplace_back(triangle[2]);
        line.clear();
    }

    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    /*
     *  normal
     */
    for (size_t i = 0; i < mBodyVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "     %f;%f;%f;%c", &pos[0], &pos[1], &pos[2], &ignore);
        mBodyNormalList.emplace_back(pos[0], pos[1], pos[2]);
        line.clear();
    }

    std::getline(stream, line);
    line.clear();

    for (size_t i = 0; i < mBodyIndexListCount; ++i)
    {
        std::getline(stream, line);
        line.clear();
    }
    /*
     *
     * Face
     *
     */
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    std::getline(stream, line);
    sscanf_s((char*)line.c_str(), "    %d;", &mFaceVertexCount);
    line.clear();

    /*
     * vertex position
     */
    for (size_t i = 0; i < mFaceVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "    %f;%f;%f;%c", &pos[0], &pos[1], &pos[2], &ignore);
        mFacePositionList.emplace_back(pos[0], pos[1], pos[2]);
        line.clear();
    }

    /*
     *  index list
     */
    std::getline(stream, line);
    sscanf_s(line.c_str(), "    %d;", &mFaceIndexListCount);
    line.clear();

    for (size_t i = 0; i < mFaceIndexListCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "    3;%d,%d,%d;%c", &triangle[0], &triangle[1], &triangle[2], &ignore);
        mFaceIndexList.emplace_back(triangle[0]);
        mFaceIndexList.emplace_back(triangle[1]);
        mFaceIndexList.emplace_back(triangle[2]);
        line.clear();
    }

    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    /*
     *  normal
     */
    for (size_t i = 0; i < mFaceVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "     %f;%f;%f;%c", &pos[0], &pos[1], &pos[2], &ignore);
        mFaceNormalList.emplace_back(pos[0], pos[1], pos[2]);
        line.clear();
    }

    std::getline(stream, line);
    line.clear();

    for (size_t i = 0; i < mFaceIndexListCount; ++i)
    {
        std::getline(stream, line);
        line.clear();
    }

    /*
     *
     * Hair
     *
     */
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    std::getline(stream, line);
    sscanf_s((char*)line.c_str(), "    %d;", &mHairVertexCount);
    line.clear();

    /*
     * vertex position
     */
    for (size_t i = 0; i < mHairVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "    %f;%f;%f;%c", &pos[0], &pos[1], &pos[2], &ignore);
        mHairPositionList.emplace_back(pos[0], pos[1], pos[2]);
        line.clear();
    }

    /*
     *  index list
     */
    std::getline(stream, line);
    sscanf_s(line.c_str(), "    %d;", &mHairIndexListCount);
    line.clear();

    for (size_t i = 0; i < mHairIndexListCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "    3;%d,%d,%d;%c", &triangle[0], &triangle[1], &triangle[2], &ignore);
        mHairIndexList.emplace_back(triangle[0]);
        mHairIndexList.emplace_back(triangle[1]);
        mHairIndexList.emplace_back(triangle[2]);
        line.clear();
    }

    std::getline(stream, line);
    std::getline(stream, line);
    line.clear();

    /*
     *  normal
     */
    for (size_t i = 0; i < mHairVertexCount; ++i)
    {
        std::getline(stream, line);
        sscanf_s(line.c_str(), "     %f;%f;%f;%c", &pos[0], &pos[1], &pos[2], &ignore);
        mHairNormalList.emplace_back(pos[0], pos[1], pos[2]);
        line.clear();
    }


    stream.close();
}

size_t ModelImporter::GetVertexCount() const
{
    return mVertexCount;
}

size_t ModelImporter::GetIndexListCount() const
{
    return mIndexListCount * 3;
}

void ModelImporter::GetIndexList(unsigned int* indexList) const
{
    ASSERT(indexList != nullptr, "indexList가 nullptr입니다. mIndexListCount만큼 할당된 메모리를 전달해야합니다.");
    //size_t count = sizeof(**indexList) / sizeof(WORD);
    //ASSERT(count == mIndexListCount, "indexList의 크기가 mIndexListCount와 같지 않습니다. mIndexListCount만큼 할당된 메모리를 전달해야합니다.");

    for(size_t i = 0; i < mIndexListCount*3; ++i)
    {
        *(indexList+i) = mIndexList[i];
    }

    //memcpy(*indexList, mIndexList.data(), sizeof(WORD)*mIndexListCount);
}

void ModelImporter::GetVertexData(const size_t index, XMFLOAT3& pos, XMFLOAT2& tex, XMFLOAT3& norm) const
{
    pos = mPositionList[index];
    tex = mTexUVList[index];
    norm = mNormalList[index];
}

void ModelImporter::StrSplit(const char* str, const char* delim, char** outStrToStore, const size_t& outArraySize)
{
    ASSERT(str != nullptr, "str이 nullptr입니다.");
    ASSERT(delim != nullptr, "delim이 nullptr입니다.");
    ASSERT(outStrToStore != nullptr, "outStrSplited이 nullptr입니다.");
    ASSERT(outArraySize < 1, "배열의 크기는 1이상이어야 합니다.");
    
    size_t numSplited = 0;

    const char* begin = str;
    const char* beginDelim = delim;
    while(*str++ != '\0')
    {
        beginDelim = delim;
        while(*beginDelim++ != '\0')
        {
            if (*str == *beginDelim)
            {
                outStrToStore[numSplited] = new char[(str-begin)+1];
                memcpy_s(outStrToStore[numSplited], (str - begin) + 1, str, str - begin);
                outStrToStore[numSplited++][str - begin] = '\0';
                begin = str + 1;
                if(numSplited >= outArraySize)
                {
                    ASSERT(false, "분리할 수 있는 문자가 남아있으나, outArraySize크기가 부족합니다. 필요하다면 더 큰 outArraySize를 전달하세요.");
                    goto END_SPLIT;
                }
            }
        }
    }

    outStrToStore[numSplited] = new char[(str - begin) + 1];
    memcpy_s(outStrToStore[numSplited++], (str - begin) + 1, str, str - begin);
    outStrToStore[numSplited][str - begin] = '\0';

    END_SPLIT:
    return;
}