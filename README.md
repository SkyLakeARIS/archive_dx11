# ModelViewerDx11

DirectX 11로 렌더 커맨드 큐 구조를 구현한 렌더러입니다.
객체가 직접 그리지 않고 커맨드를 구성해 리스트에 담고,
SortKey 기준으로 정렬한 뒤 직전 상태와 비교해 달라진 것만 바인딩합니다.

현재 진행 중인 개인 프로젝트입니다.

---

## 코드 위치

| 내용 | 파일 |
|---|---|
| RenderPacket 정의, SortKey 생성(팩토리) | `Renderer/Resources/RenderPacket.h` |
| 상태 캐시 (RenderPacketCache) | `Renderer/Resources/RenderPacket.h` |
| 커맨드 수집 · 정렬 · 소비 루프 | `Application.cpp` |
| 정점/인덱스 버퍼 청크 관리 | `Renderer/Resources/BufferManager.h` |
| 셰이더 로드 및 바인드 슬롯 테이블 | `Renderer/Shader/ShaderManager.h` |
| 렌더 상태 열거형, BufferRange | `Renderer/Resources/RenderTypes.h` |

---

## 구조 요약

```
Application          메인 루프 · 커맨드 리스트 소유
├── Core             Window · DirectInput · Timer
├── Scene            Camera · Light · Sky · Floor · Billboard
├── UI               DebugPanel
└── Renderer         Device · SwapChain · 상태 바인드
    ├── Resources    ResourceManager · BufferManager · TextureManager
    │                RenderPacket · Model · Mesh · Material
    ├── Shader       ShaderManager
    ├── Importer     ModelImporter (FBX)
    └── Primitive    MeshGenerator
```

---

## SortKey

64bit. 값이 클수록 먼저 그려지도록 하고 내림차순 정렬합니다.
교체 비용이 큰 상태일수록 상위 비트에 배치했습니다.

```
63          54          35        20   18      0
│           │           │         │    │       │
└ RenderTarget          └ Texture Serial       └ Shader / VF / Raster / Topology
            └ Transparency
```

| 구간 | 기준 |
|---|---|
| 상위 | 순서가 중요한 값. 렌더타겟(Pass), 투명 여부 |
| 중위 | 교체 비용이 커서 덜 바뀌어야 하는 값. 텍스처, (예정) 머티리얼 |
| 하위 | 자잘한 렌더 상태. 교체 비용이 상대적으로 작음 |

32bit로 시작했으나 텍스처 식별자를 추가하며 64bit로 확장했습니다.

---

## 빌드

- Visual Studio 2022
- `ModelViewerDx11.sln`
- FBX SDK
- DirectXTex (NuGet)