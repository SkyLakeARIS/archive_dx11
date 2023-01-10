#include "ModelImporter.h"
#include <fstream>
#include <string>

ModelImporter::ModelImporter()
    : mBodyVertexCount(0)
    , mBodyIndexListCount(0)
    , mFaceVertexCount(0)
    , mFaceIndexListCount(0)
    , mHairVertexCount(0)
    , mHairIndexListCount(0)
{
    mBodyPositionList.reserve(32768);
    mBodyNormalList.reserve(32768);
    mBodyIndexList.reserve(80000);

    mFacePositionList.reserve(32768);
    mFaceNormalList.reserve(32768);
    mFaceIndexList.reserve(80000);

    mHairPositionList.reserve(32768);
    mHairNormalList.reserve(32768);
    mHairIndexList.reserve(80000);
}

ModelImporter::~ModelImporter()
{
}


void ModelImporter::LoadModelAnby()
{
    std::ifstream stream;

    stream.open("C:\\Users\\bunke\\바탕 화면\\test_model2.x");
    if (!stream.is_open())
    {
        ASSERT(false, "test_model2 파일 열기 실패");
        return;
    }

    mMatRootTM = XMMATRIX(
        39.370080, 0.000000, 0.000000, 0.000000,
        0.000000, -0.000002, -39.370080, 0.000000,
        0.000000, 39.370080, -0.000002, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000);

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

    /*
     *  중복데이터 건너뜀
     */
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

    /*
     *  중복 데이터 건너뜀.
     */
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

    // 
    mBodyIndexListCount *= 3;
    mFaceIndexListCount *= 3;
    mHairIndexListCount *= 3;

    stream.close();
}
