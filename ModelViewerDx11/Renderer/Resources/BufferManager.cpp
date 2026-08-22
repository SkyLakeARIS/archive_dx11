#include "BufferManager.h"
#include <algorithm>
#include <stack>
#include "RenderTypes.h"
#include "../../Util/Macro.h"

namespace renderer
{
    bool BufferRangeIncrCompare(const BufferRange& left, const BufferRange& right)
    {
        return left.StartIndex < right.StartIndex;
    }

    BufferManager::IndexFormatPair BufferManager::sIndexFormatMap[static_cast<int8_t>(BufferManager::eIndexListFormat::IndexListFormatCount)] =
    {
        {sizeof(uint32_t), DXGI_FORMAT_R32_UINT},
    };

    BufferManager::BufferManager(ID3D11Device* device, ID3D11DeviceContext* deviceContext, eIndexListFormat indexFormat)
        : mDevice(device)
        , mDeviceContext(deviceContext)
        , mIndexFormat(indexFormat)
    {
        ASSERT(mDevice, "device is nullptr. pass the valid device");
        ASSERT(mDeviceContext, "deviceContext is nullptr. pass the valid deviceContext");
    }

    BufferManager::~BufferManager()
    {
        mDevice = nullptr;
        mDeviceContext = nullptr;

        for (auto& buffer : mVertexBuffers)
        {
            SAFETY_RELEASE(buffer.second.Buffer);
            buffer.second.SubChunks.clear();
        }
        mVertexBuffers.clear();

        for (auto& strideIt : mVertexRemovedRanges)
        {
            strideIt.second.clear();
            std::vector<BufferRange>().swap(strideIt.second);
        }
        mVertexRemovedRanges.clear();

        for (auto& buffer : mIndexBuffers)
        {
            SAFETY_RELEASE(buffer.second.Buffer);
            buffer.second.SubChunks.clear();
        }
        mIndexBuffers.clear();

        for (auto& strideIt : mIndexRemovedRanges)
        {
            strideIt.second.clear();
            std::vector<BufferRange>().swap(strideIt.second);
        }
        mIndexRemovedRanges.clear();


        for (auto& buffer : mVertexBuffersDynamic)
        {
            SAFETY_RELEASE(buffer.second.Buffer);
            buffer.second.SubChunks.clear();
        }
        mVertexBuffersDynamic.clear();

        for (auto& buffer : mIndexBuffersDynamic)
        {
            SAFETY_RELEASE(buffer.second.Buffer);
            buffer.second.SubChunks.clear();
        }
        mIndexBuffersDynamic.clear();
    }

    bool BufferManager::Initialize(int32_t vertexBufferByteSizeStatic, int32_t indexBufferByteSizeStatic, int32_t vertexBufferByteSizeDynamic, int32_t indexBufferByteSizeDynamic)
    {
        if(vertexBufferByteSizeStatic < 0)
        {
            ASSERT(vertexBufferByteSizeStatic >= 0, "vtx buf size must over 0.");
            return false;
        }

        if (indexBufferByteSizeStatic < 0)
        {
            ASSERT(indexBufferByteSizeStatic >= 0, "idx buf size must over 0.");
            return false;
        }


        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = vertexBufferByteSizeStatic;



        mVertexBuffers.reserve(static_cast<int16_t>(eVertexFormat::FormatCount));
        for(int16_t layoutType = 0; layoutType < static_cast<int16_t>(eVertexFormat::FormatCount); ++layoutType)
        {
            const int16_t stride = GetVertexStrideSize(static_cast<eVertexFormat>(layoutType));
            BufferChunk bufferRes = {};
            if (mDevice->CreateBuffer(&bufferDesc, nullptr, &bufferRes.Buffer) == E_FAIL)
            {
                ASSERT(false, "vertex buffer creation failed, check the options. layoutType(%d)", layoutType);
                return false;
            }
            bufferRes.SubChunks.reserve(128);
            bufferRes.TotalSizeBytes = vertexBufferByteSizeStatic;
            mVertexBuffers.insert(std::make_pair(stride, std::move(bufferRes)));
            std::vector<BufferRange> ranges;
            ranges.reserve(128);
            mVertexRemovedRanges.insert(std::make_pair(stride, std::move(ranges)));
        }


        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.ByteWidth = indexBufferByteSizeStatic;

        mIndexBuffers.reserve(static_cast<int16_t>(eVertexFormat::FormatCount));
        const int16_t indexStrideSize = sIndexFormatMap[static_cast<int8_t>(mIndexFormat)].Stride;


        BufferChunk indexBufResStatic = {};
        indexBufResStatic.SubChunks.reserve(256);
        if (mDevice->CreateBuffer(&bufferDesc, nullptr, &indexBufResStatic.Buffer) == E_FAIL)
        {
            ASSERT(false, "index buffer creation failed, check the options. layoutType(%d)", indexStrideSize);
            return false;
        }
        indexBufResStatic.TotalSizeBytes = indexBufferByteSizeStatic;
        mIndexBuffers.insert(std::make_pair(indexStrideSize, std::move(indexBufResStatic)));
        std::vector<BufferRange> ranges;
        ranges.reserve(256);
        mIndexRemovedRanges.insert(std::make_pair(indexStrideSize, std::move(ranges)));

        // init Dynamic Buffers
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = vertexBufferByteSizeDynamic;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        mVertexBuffersDynamic.reserve(static_cast<int16_t>(eVertexFormat::FormatCount));
        for (int16_t layoutType = 0; layoutType < static_cast<int16_t>(eVertexFormat::FormatCount); ++layoutType)
        {
            const int16_t stride = GetVertexStrideSize(static_cast<eVertexFormat>(layoutType));
            BufferChunk bufferResDynamic = {};
            if (mDevice->CreateBuffer(&bufferDesc, nullptr, &bufferResDynamic.Buffer) == E_FAIL)
            {
                ASSERT(false, "vertex buffer(dynamic) creation failed, check the options. layoutType(%d)", layoutType);
                return false;
            }
            bufferResDynamic.SubChunks.reserve(16);
            bufferResDynamic.TotalSizeBytes = vertexBufferByteSizeDynamic;
            mVertexBuffersDynamic.insert(std::make_pair(stride, std::move(bufferResDynamic)));
        }


        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.ByteWidth = indexBufferByteSizeDynamic;

        mIndexBuffersDynamic.reserve(static_cast<int16_t>(eVertexFormat::FormatCount));

        BufferChunk indexBufDynamic = {};
        if (mDevice->CreateBuffer(&bufferDesc, nullptr, &indexBufDynamic.Buffer) == E_FAIL)
        {
            ASSERT(false, "index buffer(dynamic) creation failed, check the options. layoutType(%d)", indexStrideSize);
            return false;
        }
        indexBufDynamic.SubChunks.reserve(32);
        indexBufDynamic.TotalSizeBytes = indexBufferByteSizeDynamic;
        mIndexBuffersDynamic.insert(std::make_pair(indexStrideSize, std::move(indexBufDynamic)));
        std::vector<BufferRange> rangesDynamic;
        rangesDynamic.reserve(32);

        return true;
    }

    void BufferManager::AddVertex(const int8_t* const pData, int32_t dataByteSize, HashID hash, int16_t stride, BufferRange& outRangeInBuffer)
    {
        ASSERT(pData, "pData is nullptr,");
        ASSERT(dataByteSize > 0, "dataByteSize is zero or negative");
        ASSERT(stride > 0, "stride is zero or negative");
        if (!pData || dataByteSize <= 0)
        {
            return;
        }

        auto chunkIt = mVertexBuffers.find(stride);

        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if(subChunkIt != chunkIt->second.SubChunks.end())
        {
            // MEMO: in vertex count (not bytes). convert bytes -> stride
            outRangeInBuffer.Count = subChunkIt->second.Ranges.Count / chunkIt->first;
            outRangeInBuffer.StartIndex = subChunkIt->second.Ranges.StartIndex / chunkIt->first;
            ++subChunkIt->second.RefCount;
            return;
        }

        // MEMO: 적절한 공간을 가진 빈공간 탐색
        const auto& removedRangeIt = mVertexRemovedRanges.find(stride);
        std::vector<BufferRange>::iterator bestFitSpaceIt = removedRangeIt->second.end();
        int32_t minRemainSpace = INT32_MAX;
        for (auto rangeIt = removedRangeIt->second.begin(); rangeIt != removedRangeIt->second.end(); ++rangeIt)
        {
            const int32_t remainSpace = rangeIt->Count - dataByteSize;
            if (remainSpace >= 0)
            {
                // MEMO: save best-fit.
                if (remainSpace < minRemainSpace)
                {
                    minRemainSpace = remainSpace;
                    bestFitSpaceIt = rangeIt;
                }
            }
        }

        int32_t writeCursorInBuffer  = chunkIt->second.CursorBytes;
        if(bestFitSpaceIt != removedRangeIt->second.end())
        {
            ASSERT(dataByteSize <= bestFitSpaceIt->Count, "재사용 로직 에러. 올바르지 않은 요소가 선택 됨. dataByteSize(%d), bestFitSize(%d)", dataByteSize, bestFitSpaceIt->Count);
            // MEMO: 빈공간 재활용
            if(minRemainSpace == 0)
            {
                *bestFitSpaceIt = removedRangeIt->second.back();
                removedRangeIt->second.pop_back();
            }
            else
            {
                writeCursorInBuffer = bestFitSpaceIt->StartIndex;
                // MEMO: 재활용하고 남은 공간은 또 재활용을 하기 위함.
                bestFitSpaceIt->StartIndex = bestFitSpaceIt->StartIndex + dataByteSize;
                bestFitSpaceIt->Count = minRemainSpace;
            }
        }
        else
        {
            // MEMO: 재활용할 공간이 없음
            if (chunkIt->second.TotalSizeBytes <= writeCursorInBuffer + dataByteSize)
            {
                resizeBuffer(writeCursorInBuffer + dataByteSize, D3D11_BIND_VERTEX_BUFFER,  D3D11_USAGE_DEFAULT, 0, chunkIt);
            }

            chunkIt->second.CursorBytes += dataByteSize;
        }

        D3D11_BOX updateRange = {};
        updateRange.front = 0;
        updateRange.back = 1;
        updateRange.top = 0;
        updateRange.bottom = 1;
        updateRange.left = writeCursorInBuffer;
        updateRange.right = writeCursorInBuffer + dataByteSize;
        mDeviceContext->UpdateSubresource(chunkIt->second.Buffer, 0, &updateRange, pData, 0, 0);

        SubChunk subChunk = {};
        subChunk.Ranges.StartIndex = writeCursorInBuffer;
        subChunk.Ranges.Count = dataByteSize;
        subChunk.RefCount = 1;

        chunkIt->second.SubChunks.insert(std::make_pair(hash, subChunk));

        // MEMO: in vertex count (not bytes). convert bytes -> stride
        outRangeInBuffer.Count = subChunk.Ranges.Count / chunkIt->first;
        outRangeInBuffer.StartIndex = subChunk.Ranges.StartIndex / chunkIt->first;
    }

    void BufferManager::AddIndex(const int8_t* const pData, int32_t dataByteSize, HashID hash, int16_t stride, BufferRange& outRangeInBuffer)
    {
        ASSERT(pData, "pData is nullptr,");
        ASSERT(dataByteSize > 0, "dataByteSize is zero or negative");
        ASSERT(stride > 0, "stride is zero or negative");
        if (!pData || dataByteSize <= 0)
        {
            return;
        }

        auto chunkIt = mIndexBuffers.find(stride);

        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt != chunkIt->second.SubChunks.end())
        {
            // MEMO: in vertex count (not bytes). convert bytes -> stride
            outRangeInBuffer.Count = subChunkIt->second.Ranges.Count / chunkIt->first;
            outRangeInBuffer.StartIndex = subChunkIt->second.Ranges.StartIndex / chunkIt->first;
            ++subChunkIt->second.RefCount;
            return;
        }

        // MEMO: 적절한 공간을 가진 빈공간 탐색
        const auto& removedRangeIt = mIndexRemovedRanges.find(stride);
        std::vector<BufferRange>::iterator bestFitSpaceIt = removedRangeIt->second.end();
        int32_t minRemainSpace = INT32_MAX;
        for (auto rangeIt = removedRangeIt->second.begin(); rangeIt != removedRangeIt->second.end(); ++rangeIt)
        {
            const int32_t remainSpace = rangeIt->Count - dataByteSize;
            if (remainSpace >= 0)
            {
                // MEMO: save best-fit.
                if (remainSpace < minRemainSpace)
                {
                    minRemainSpace = remainSpace;
                    bestFitSpaceIt = rangeIt;
                }
            }
        }

        int32_t writeCursorInBuffer = chunkIt->second.CursorBytes;
        if (bestFitSpaceIt != removedRangeIt->second.end())
        {
            ASSERT(dataByteSize <= bestFitSpaceIt->Count, "재사용 로직 에러. 올바르지 않은 요소가 선택 됨. dataByteSize(%d), bestFitSize(%d)", dataByteSize, bestFitSpaceIt->Count);
            // MEMO: 빈공간 재활용
            if (minRemainSpace == 0)
            {
                *bestFitSpaceIt = removedRangeIt->second.back();
                removedRangeIt->second.pop_back();
            }
            else
            {
                writeCursorInBuffer = bestFitSpaceIt->StartIndex;
                // MEMO: 재활용하고 남은 공간은 또 재활용을 하기 위함.
                bestFitSpaceIt->StartIndex = bestFitSpaceIt->StartIndex + dataByteSize;
                bestFitSpaceIt->Count = minRemainSpace;
            }
        }
        else
        {
            // MEMO: 재활용할 공간이 없음
            if (chunkIt->second.TotalSizeBytes <= writeCursorInBuffer + dataByteSize)
            {
                resizeBuffer(writeCursorInBuffer + dataByteSize, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DEFAULT, 0, chunkIt);
            }

            chunkIt->second.CursorBytes += dataByteSize;
        }

        D3D11_BOX updateRange = {};
        updateRange.front = 0;
        updateRange.back = 1;
        updateRange.top = 0;
        updateRange.bottom = 1;
        updateRange.left = writeCursorInBuffer;
        updateRange.right = writeCursorInBuffer + dataByteSize;
        mDeviceContext->UpdateSubresource(chunkIt->second.Buffer, 0, &updateRange, pData, 0, 0);

        SubChunk subChunk = {};
        subChunk.Ranges.StartIndex = writeCursorInBuffer;
        subChunk.Ranges.Count = dataByteSize;
        subChunk.RefCount = 1;

        chunkIt->second.SubChunks.insert(std::make_pair(hash, subChunk));

        // MEMO: in vertex count (not bytes). convert bytes -> stride
        outRangeInBuffer.Count = subChunk.Ranges.Count / chunkIt->first;
        outRangeInBuffer.StartIndex = subChunk.Ranges.StartIndex / chunkIt->first;
    }

    void BufferManager::AddVertexDynamic(const int8_t* const pData, int32_t dataByteSize, HashID hash, int16_t stride, BufferRange& outRangeInBuffer)
    {
        ASSERT(pData, "pData is nullptr,");
        ASSERT(dataByteSize > 0, "dataByteSize is zero or negative");
        ASSERT(stride > 0, "stride is zero or negative");
        if (!pData || dataByteSize <= 0)
        {
            return;
        }

        auto chunkIt = mVertexBuffersDynamic.find(stride);

        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt != chunkIt->second.SubChunks.end())
        {
            // MEMO: in vertex count (not bytes). convert bytes -> stride
            outRangeInBuffer.Count = subChunkIt->second.Ranges.Count / chunkIt->first;
            outRangeInBuffer.StartIndex = subChunkIt->second.Ranges.StartIndex / chunkIt->first;
            return;
        }

        if (chunkIt->second.TotalSizeBytes <= chunkIt->second.CursorBytes + dataByteSize)
        {
            resizeVertexBuffer(chunkIt->second.CursorBytes + dataByteSize, D3D11_USAGE_DYNAMIC, 0, chunkIt);
        }

        const D3D11_MAP mapType = mbNeedDiscardDynamicVertex ? (D3D11_MAP_WRITE_DISCARD) : D3D11_MAP_WRITE_NO_OVERWRITE;
        mbNeedDiscardDynamicVertex = false;

        D3D11_MAPPED_SUBRESOURCE mappedRes = {};
        mDeviceContext->Map(chunkIt->second.Buffer, 0, mapType, 0, &mappedRes);

        int8_t* gpuBuffer = reinterpret_cast<int8_t*>(mappedRes.pData);
        memcpy(gpuBuffer + chunkIt->second.CursorBytes, pData, dataByteSize);

        mDeviceContext->Unmap(chunkIt->second.Buffer, 0);

        SubChunk subChunk = {};
        subChunk.Ranges.StartIndex = chunkIt->second.CursorBytes;
        subChunk.Ranges.Count = dataByteSize;
        // MEMO: 동적 데이터에는 필요 없음.

        chunkIt->second.CursorBytes += dataByteSize;
        chunkIt->second.SubChunks.insert(std::make_pair(hash, subChunk));

        // MEMO: in vertex count (not bytes). convert bytes -> stride
        outRangeInBuffer.Count = subChunk.Ranges.Count / chunkIt->first;
        outRangeInBuffer.StartIndex = subChunk.Ranges.StartIndex / chunkIt->first;
    }

    void BufferManager::AddIndexDynamic(const int8_t* const pData, int32_t dataByteSize, HashID hash, int16_t stride,
        BufferRange& outRangeInBuffer)
    {
        ASSERT(pData, "pData is nullptr,");
        ASSERT(dataByteSize > 0, "dataByteSize is zero or negative");
        ASSERT(stride > 0, "stride is zero or negative");
        if (!pData || dataByteSize <= 0)
        {
            return;
        }

        auto chunkIt = mIndexBuffersDynamic.find(stride);

        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt != chunkIt->second.SubChunks.end())
        {
            // MEMO: in vertex count (not bytes). convert bytes -> stride
            outRangeInBuffer.Count = subChunkIt->second.Ranges.Count / chunkIt->first;
            outRangeInBuffer.StartIndex = subChunkIt->second.Ranges.StartIndex / chunkIt->first;
            return;
        }

        if (chunkIt->second.TotalSizeBytes <= chunkIt->second.CursorBytes + dataByteSize)
        {
            resizeIndexBuffer(chunkIt->second.CursorBytes + dataByteSize, D3D11_USAGE_DYNAMIC, 0, chunkIt);
        }


        const D3D11_MAP mapType = mbNeedDiscardDynamicIndex ? (D3D11_MAP_WRITE_DISCARD) : D3D11_MAP_WRITE_NO_OVERWRITE;
        mbNeedDiscardDynamicIndex = false;

        D3D11_MAPPED_SUBRESOURCE mappedRes = {};
        mDeviceContext->Map(chunkIt->second.Buffer, 0, mapType, 0, &mappedRes);

        int8_t* gpuBuffer = reinterpret_cast<int8_t*>(mappedRes.pData);
        memcpy(gpuBuffer + subChunkIt->second.Ranges.StartIndex, pData, dataByteSize);

        mDeviceContext->Unmap(chunkIt->second.Buffer, 0);

        SubChunk subChunk = {};
        subChunk.Ranges.StartIndex = chunkIt->second.CursorBytes;
        subChunk.Ranges.Count = dataByteSize;
        // MEMO: 동적 데이터에는 필요 없음.

        chunkIt->second.SubChunks.insert(std::make_pair(hash, subChunk));
        chunkIt->second.CursorBytes += dataByteSize;

        // MEMO: in vertex count (not bytes). convert bytes -> stride
        outRangeInBuffer.Count = subChunk.Ranges.Count / chunkIt->first;
        outRangeInBuffer.StartIndex = subChunk.Ranges.StartIndex / chunkIt->first;
    }

    void BufferManager::RemoveVertexData(int16_t stride, HashID hash)
    {
        ASSERT(stride > 0, "stride is zero or negative");
        ASSERT(hash > 0, "hash is zero or negative");

        const auto& chunkIt = mVertexBuffers.find(stride);
        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if(subChunkIt != chunkIt->second.SubChunks.end())
        {
            --subChunkIt->second.RefCount;
            if (subChunkIt->second.RefCount <= 0)
            {
                const auto& removedRangeIt = mVertexRemovedRanges.find(stride);
                removedRangeIt->second.push_back(subChunkIt->second.Ranges);

                chunkIt->second.SubChunks.erase(subChunkIt);
                // MEMO: 연속된 빈공간 병합 시도
                if (removedRangeIt->second.size() >= 2)
                {
                    mergeRemovedSpace(removedRangeIt);
                }
            }
        }
    }

    void BufferManager::RemoveIndexData(int16_t stride, HashID hash)
    {
        ASSERT(stride > 0, "stride is zero or negative");
        ASSERT(hash > 0, "hash is zero or negative");

        const auto& chunkIt = mIndexBuffers.find(stride);
        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt != chunkIt->second.SubChunks.end())
        {
            --subChunkIt->second.RefCount;
            if(subChunkIt->second.RefCount <= 0)
            {
                const auto& removedRangeIt = mIndexRemovedRanges.find(stride);
                removedRangeIt->second.push_back(subChunkIt->second.Ranges);

                chunkIt->second.SubChunks.erase(subChunkIt);
                if (removedRangeIt->second.size() >= 2)
                {
                    mergeRemovedSpace(removedRangeIt);
                }
            }
        }
    }

    void BufferManager::UpdateVertexData(int8_t* const pData, int16_t stride, HashID hash)
    {
        ASSERT(stride > 0, "stride is zero or negative");
        ASSERT(hash > 0, "hash is zero or negative");
        ASSERT(pData, "pData is nullptr. nullptr이 아닌 유효한 pData를 전달해야 합니다.");

        const auto& chunkIt = mVertexBuffers.find(stride);
        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt == chunkIt->second.SubChunks.end())
        {
            return;
        }

        // MEMO: 업데이트이므로 기존에 삽입된 사이즈만큼의 데이터를 복사하도록 함.
        // 현재 구조에서는 사이즈를 키우거나 줄이려면, 제거 후 다시 추가해야 함.
        D3D11_BOX updateRange = {};
        updateRange.front = 0;
        updateRange.back = 1;
        updateRange.top = 0;
        updateRange.bottom = 1;
        updateRange.left = subChunkIt->second.Ranges.StartIndex;
        updateRange.right = subChunkIt->second.Ranges.StartIndex + subChunkIt->second.Ranges.Count;
        mDeviceContext->UpdateSubresource(chunkIt->second.Buffer, 0, &updateRange, pData, 0, 0);
    }

    void BufferManager::UpdateIndexData(int8_t* const pData, int16_t stride, HashID hash)
    {
        ASSERT(stride > 0, "stride is zero or negative");
        ASSERT(hash > 0, "hash is zero or negative");
        ASSERT(pData, "pData is nullptr. nullptr이 아닌 유효한 pData를 전달해야 합니다.");

        const auto& chunkIt = mIndexBuffers.find(stride);
        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt == chunkIt->second.SubChunks.end())
        {
            return;
        }
        
        D3D11_BOX updateRange = {};
        updateRange.front = 0;
        updateRange.back = 1;
        updateRange.top = 0;
        updateRange.bottom = 1;
        updateRange.left = subChunkIt->second.Ranges.StartIndex;
        updateRange.right = subChunkIt->second.Ranges.StartIndex + subChunkIt->second.Ranges.Count;
        mDeviceContext->UpdateSubresource(chunkIt->second.Buffer, 0, &updateRange, pData, 0, 0);
    }

    void BufferManager::MarkInvalidateDynamicBuf()
    {
        mbNeedDiscardDynamicVertex = true;

        for (auto& bufStrideIt : mVertexBuffersDynamic)
        {
            bufStrideIt.second.CursorBytes = 0;
            bufStrideIt.second.SubChunks.clear();
        }

        mbNeedDiscardDynamicIndex = true;
        for (auto& bufStrideIt : mIndexBuffersDynamic)
        {
            bufStrideIt.second.CursorBytes = 0;
            bufStrideIt.second.SubChunks.clear();
        }
    }

    BufferRange BufferManager::GetVertexRangeByteByHash(int16_t stride, HashID hash)
    {
        ASSERT(stride > 0, "stride is zero or negative");
        ASSERT(hash > 0, "hash is zero or negative");
        BufferRange range = { -1, -1 };

        const auto& chunkIt = mVertexBuffers.find(stride);
        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt != chunkIt->second.SubChunks.end())
        {
            range = subChunkIt->second.Ranges;
        }

        return range;
    }

    BufferRange BufferManager::GetIndexRangeByteByHash(int16_t stride, HashID hash)
    {
        ASSERT(stride > 0, "stride is zero or negative");
        ASSERT(hash > 0, "hash is zero or negative");
        BufferRange range = { -1, -1 };

        const auto& chunkIt = mIndexBuffers.find(stride);
        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt != chunkIt->second.SubChunks.end())
        {
            range = subChunkIt->second.Ranges;
        }

        return range;
    }

    BufferRange BufferManager::GetVertexRangeCountByHash(int16_t stride, HashID hash)
    {
        ASSERT(stride > 0, "stride is zero or negative");
        ASSERT(hash > 0, "hash is zero or negative");
        BufferRange range = { -1, -1 };

        const auto& chunkIt = mVertexBuffers.find(stride);
        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt != chunkIt->second.SubChunks.end())
        {
            range.Count = subChunkIt->second.Ranges.Count / chunkIt->first;
            range.StartIndex = subChunkIt->second.Ranges.StartIndex / chunkIt->first;
        }

        return range;
    }

    BufferRange BufferManager::GetIndexRangeCountByHash(int16_t stride, HashID hash)
    {
        ASSERT(stride > 0, "stride is zero or negative");
        ASSERT(hash > 0, "hash is zero or negative");
        BufferRange range = { -1, -1 };

        const auto& chunkIt = mIndexBuffers.find(stride);
        const auto& subChunkIt = chunkIt->second.SubChunks.find(hash);
        if (subChunkIt != chunkIt->second.SubChunks.end())
        {
            range.Count = subChunkIt->second.Ranges.Count / chunkIt->first;
            range.StartIndex = subChunkIt->second.Ranges.StartIndex / chunkIt->first;
        }

        return range;
    }

    ID3D11Buffer* BufferManager::GetVertexBuffer(int16_t stride) const
    {
        const auto& chunkIt = mVertexBuffers.find(stride);
        return chunkIt->second.Buffer;
    }

    ID3D11Buffer* BufferManager::GetIndexBuffer(int16_t stride) const
    {
        const auto& chunkIt = mIndexBuffers.find(stride);
        return chunkIt->second.Buffer;
    }

    ID3D11Buffer* BufferManager::GetVertexBufferDynamic(int16_t stride) const
    {
        const auto& chunkIt = mVertexBuffersDynamic.find(stride);
        return chunkIt->second.Buffer;
    }

    ID3D11Buffer* BufferManager::GetIndexBufferDynamic(int16_t stride) const
    {
        const auto& chunkIt = mIndexBuffersDynamic.find(stride);
        return chunkIt->second.Buffer;
    }

    int16_t BufferManager::GetIndexStrideSize() const
    {
        return sIndexFormatMap[static_cast<int8_t>(mIndexFormat)].Stride;
    }

    DXGI_FORMAT BufferManager::GetIndexFormat() const
    {
        return sIndexFormatMap[static_cast<int8_t>(mIndexFormat)].Format;
    }

    void BufferManager::resizeVertexBuffer(uint32_t newSize, D3D11_USAGE usageType, uint32_t cpuAccessFlag, std::unordered_map<int16_t, BufferChunk>::iterator& chunkIt)
    {
        ID3D11Buffer* resizedBuffer = nullptr;
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.Usage = usageType;
        bufferDesc.ByteWidth = newSize * 2;
        bufferDesc.CPUAccessFlags = cpuAccessFlag;
        if (mDevice->CreateBuffer(&bufferDesc, nullptr, &resizedBuffer) == E_FAIL)
        {
            ASSERT(false, "vertex buffer creation failed while resizing. check the options. tried buffer type (%d)", bufferDesc.BindFlags);
            return;
        }

        if (chunkIt->second.CursorBytes > 0)
        {
            D3D11_BOX updateRange = {};
            updateRange.front = 0;
            updateRange.back = 1;
            updateRange.top = 0;
            updateRange.bottom = 1;
            updateRange.left = 0;
            updateRange.right = chunkIt->second.CursorBytes;
            mDeviceContext->CopySubresourceRegion(resizedBuffer, 0, 0, 0, 0, chunkIt->second.Buffer, 0, &updateRange);
        }

        std::swap(chunkIt->second.Buffer, resizedBuffer);
        SAFETY_RELEASE(resizedBuffer);
        chunkIt->second.TotalSizeBytes = bufferDesc.ByteWidth;
    }

    void BufferManager::resizeIndexBuffer(uint32_t newSize, D3D11_USAGE usageType, uint32_t cpuAccessFlag, std::unordered_map<int16_t, BufferChunk>::iterator& chunkIt)
    {
        ID3D11Buffer* resizedBuffer = nullptr;
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.Usage = usageType;
        bufferDesc.ByteWidth = newSize * 2;
        bufferDesc.CPUAccessFlags = cpuAccessFlag;
        if (mDevice->CreateBuffer(&bufferDesc, nullptr, &resizedBuffer) == E_FAIL)
        {
            ASSERT(false, "index buffer creation failed while resizing. check the options. tried buffer type (%d)", bufferDesc.BindFlags);
            return;
        }

        if (chunkIt->second.CursorBytes > 0)
        {
            D3D11_BOX updateRange = {};
            updateRange.front = 0;
            updateRange.back = 1;
            updateRange.top = 0;
            updateRange.bottom = 1;
            updateRange.left = 0;
            updateRange.right = chunkIt->second.CursorBytes;
            mDeviceContext->CopySubresourceRegion(resizedBuffer, 0, 0, 0, 0, chunkIt->second.Buffer, 0, &updateRange);
        }

        std::swap(chunkIt->second.Buffer, resizedBuffer);
        SAFETY_RELEASE(resizedBuffer);
        chunkIt->second.TotalSizeBytes = bufferDesc.ByteWidth;
    }

    void BufferManager::resizeBuffer(uint32_t newSize, uint32_t bindFlag, D3D11_USAGE usageType, uint32_t cpuAccessFlag,
        std::unordered_map<int16_t, BufferChunk>::iterator& chunkIt)
    {
        ID3D11Buffer* resizedBuffer = nullptr;
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags = bindFlag;
        bufferDesc.Usage = usageType;
        bufferDesc.ByteWidth = newSize * 2;
        bufferDesc.CPUAccessFlags = cpuAccessFlag;
        if (mDevice->CreateBuffer(&bufferDesc, nullptr, &resizedBuffer) == E_FAIL)
        {
            ASSERT(false, "buffer creation failed while resizing. check the options. tried buffer type (%d), BufferUsage(%d), size(%d), cpuFlag(%d)", bufferDesc.BindFlags, bufferDesc.Usage, bufferDesc.ByteWidth, bufferDesc.CPUAccessFlags);
            return;
        }

        if (chunkIt->second.CursorBytes > 0)
        {
            D3D11_BOX updateRange = {};
            updateRange.front = 0;
            updateRange.back = 1;
            updateRange.top = 0;
            updateRange.bottom = 1;
            updateRange.left = 0;
            updateRange.right = chunkIt->second.CursorBytes;
            mDeviceContext->CopySubresourceRegion(resizedBuffer, 0, 0, 0, 0, chunkIt->second.Buffer, 0, &updateRange);
        }

        std::swap(chunkIt->second.Buffer, resizedBuffer);
        SAFETY_RELEASE(resizedBuffer);
        chunkIt->second.TotalSizeBytes = bufferDesc.ByteWidth;
    }

    void BufferManager::mergeRemovedSpace(const std::unordered_map<int16_t, std::vector<BufferRange>>::iterator& removedBufferIt)
    {
        ASSERT(removedBufferIt->second.size() >= 2, "병합 선조건은 벡터 사이즈가 2개 이상이어야 합니다. size(%d)", static_cast<int32_t>(removedBufferIt->second.size()));
        std::sort(removedBufferIt->second.begin(), removedBufferIt->second.end(), BufferRangeIncrCompare);
        // 1. 병합되고 나서 vector size가 1개 일 때.
        // 2. nextRangeIt이 end 일 때.
        // MEMO: 병합검출과 병합된 공간 제거 과정은 병합될 수 있는 케이스에 따라서 제거 시 문제가 될 수 있음.
        // 1. 연속되지 않은 서로 다른 공간
        // 2. 중간에 연속되지 않은 공간이 끼어있고, 앞 뒤로 연속된 공간이 있는 경우 <- 제거를 따로 하면 문제 생김
        // 3. 연속된 공간만 존재하는 경우
        // 4. 빈경우
        std::stack<int32_t> removeIndices;
        int32_t pivot = 0;
        // MEMO: 앞에서 2개 이상을 보장하니 괜찮음.
        int32_t cursor = 1;
        while (cursor < static_cast<int32_t>(removedBufferIt->second.size()))
        {
            if ((removedBufferIt->second[pivot].StartIndex + removedBufferIt->second[pivot].Count) == removedBufferIt->second[cursor].StartIndex)
            {
                removedBufferIt->second[pivot].Count += removedBufferIt->second[cursor].Count;
                removeIndices.push(cursor);
            }
            else
            {
                pivot = cursor;
            }
            ++cursor;
        }
        // MEMO: 병합이 끝나고 필요 없어진 요소들 제거 (거꾸로 순회하면 제거할 때 문제될 수 있는 부분을 해결)
        while (removeIndices.empty() == false)
        {
            const int32_t removeIndex = removeIndices.top();
            removeIndices.pop();

            removedBufferIt->second[removeIndex] = removedBufferIt->second.back();
            removedBufferIt->second.pop_back();
        }
    }
}
