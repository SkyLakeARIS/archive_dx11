# ModelViewerDx11

"기반 구조를 먼저 잡고 그 위에 화려한 기능을 올리자"를 지향하며 진행 중인 프로젝트입니다.

DirectX 11로 Deferred Shading 및 Render Command Queue 구조를 구현한 렌더러입니다.
객체가 직접 그리지 않고 커맨드를 구성해 리스트에 담고,
SortKey 기준으로 정렬한 뒤 직전 상태와 비교해 달라진 것만 바인딩합니다.

---

## 코드 위치

| 내용 | 파일 |
|---|---|
| RenderPacket 정의, SortKey 생성(팩토리) | `Renderer/Resources/RenderPacket.h` |
| 상태 캐시 (RenderPacketCache) | `Renderer/Resources/RenderPacket.h` |
| 커맨드 수집 · 정렬 · 소비 루프 | `Application.cpp` |
| 렌더패스 ↔ 렌더타겟 매핑, MRT 바인딩 | `Renderer/Renderer.cpp` |
| 셰이더 로드, 머티리얼 바인딩 테이블 | `Renderer/Shader/ShaderManager.h` |
| 정점/인덱스 버퍼 청크 관리 | `Renderer/Resources/BufferManager.h` |
| 정점 포맷, BufferRange | `Renderer/Resources/VertexType.h` |
| 상수버퍼 구조체, 렌더 상태 열거형 | `Renderer/Resources/RenderTypes.h` |
| 셰이더 · 상수버퍼 종류 열거형 | `Renderer/Shader/ShaderType.h` |

---

## 구조 요약

```
Application          메인 루프 · 커맨드 리스트 소유
├── Window           윈도우 생성 · 메시지 루프
├── Core             DirectInput · Timer
├── Scene            Camera · Light · Sky · Floor · Billboard
├── UI               DebugPanel
├── Util             Define · Macro · Type
└── Renderer         Renderer (Device · SwapChain · 렌더타겟 · 상태 바인드)
    ├── Resources    ResourceManager · BufferManager · TextureManager
    │                RenderPacket · Model · Mesh · Material
    ├── Shader       ShaderManager · ShaderType · ShaderSources/*.hlsl
    ├── Importer     ModelImporter (FBX)
    └── Primitive    MeshGenerator
```

---

## 렌더 패스

`Shadow → Main(GPass) → Deferred → UI` 순으로 실행됩니다.
Deferred는 렌더러 내부에서만 쓰는 패스입니다.

| Pass | 렌더타겟 | 내용 |
|---|---|---|
| Shadow | Shadow | 라이트 시점 깊이 |
| Main (GPass) | G-Buffer 5장 (MRT) | 아래 표 |
| Deferred | 백버퍼 | G-Buffer와 셰도우맵을 합쳐 라이팅 |
| UI | 백버퍼 | 디버그 HUD |

---

## SortKey

64bit. 값이 클수록 먼저 그려지도록 하고 내림차순 정렬합니다.
순서가 중요하거나 교체 비용이 큰 상태일수록 상위 비트에 배치했습니다.

| 구간 | 기준 |
|---|---|
| 상위 | 순서가 중요한 값. 렌더패스, 투명 여부 |
| 중위 | 교체 비용이 커서 덜 바뀌어야 하는 값. 셰이더, 텍스처 |
| 하위 | 자잘한 렌더 상태. 교체 비용이 상대적으로 작음 |

정렬에 들어가는 요소 타입이 필요한 비트 수를 컴파일 타임에 계산되도록 했습니다.
값과 자동으로 계산된 비트 수를 기반으로 레이아웃에 정의된 순서대로 SortKey를 구성합니다.

---

### Deferred Shading G-Buffer 구성

GPass에서 Lighting을 위해서 5장의 RenderTarget을 사용 중입니다.

| 렌더타겟 | RGB | A |
|---|---|---|
| Color | Diffuse | 예약 |
| Normal | 월드 노멀 | 예약 |
| Position | 월드 좌표 | 라이팅 대상 여부 |
| Specular | Specular | Shininess |
| Ambient | Ambient | 예약 |

---

## 빌드

- Visual Studio 2022
- `ModelViewerDx11.sln`
- FBX SDK
- DirectXTex (NuGet)