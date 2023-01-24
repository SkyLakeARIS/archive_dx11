#include "ModelImporter.h"
#include <fstream>
#include <string>

ModelImporter::ModelImporter()
    : mFbxScene(nullptr)
    , mFbxManager(nullptr)

{
    mFbxVertexInfo.reserve(65536);
    mFbxObjects.reserve(65536);
    mFbxIndexList.reserve(80000);
}

ModelImporter::~ModelImporter()
{

}

void ModelImporter::Release()
{
    mFbxScene->Destroy();
    mFbxManager->Destroy(); 
}

void ModelImporter::LoadFbxModel()
{
    mFbxManager = FbxManager::Create();

    FbxIOSettings* setting = FbxIOSettings::Create(mFbxManager, IOSROOT);
    mFbxManager->SetIOSettings(setting);

    FbxImporter* importer = FbxImporter::Create(mFbxManager, "");
    if(!importer->Initialize("/models/anbi.fbx", -1, mFbxManager->GetIOSettings()))
    {
        char str[128];
        sprintf_s(str, "\n\n\n\nERROR : %s\n\n\n", importer->GetStatus().GetErrorString());
        OutputDebugStringA(str);
        ASSERT(false, "importer->Initialize 실패.");
        return;
    }

    mFbxScene = FbxScene::Create(mFbxManager, "myScene");
    
    importer->Import(mFbxScene);

    importer->Destroy();

    FbxNode* rootNode = mFbxScene->GetRootNode();


    // 노드들 추출
    preprocess(nullptr, rootNode);

    parseMesh();

}

void ModelImporter::preprocess(FbxNode* parent, FbxNode* current)
{
    if(current->GetCamera() || current->GetLight())
    {
        return;
    }

    ObjectNode node;
    // 메시 타입인지? 메시를 가지고 있는 노드인지?
    // 메시에서 올바르게 데이터를 뽑아내기 위해서는
    // 우선 제대로 된 데이터를 가지고 있는 노드만을 선별해야 하므로
    // 거르는 것이 필요하다.
    if (current->GetMesh())
    {
        node.current = current;
        // 현재 아직은 쓸모 없는데, 계층 구조 구성시 사용되지 않을까 예상.
        node.parent = parent;

        mFbxObjects.push_back(node);
    }

    size_t numChild = current->GetChildCount();
    for(size_t child = 0; child < numChild; ++child)
    {
        preprocess(current, current->GetChild(child));
    }
}

void ModelImporter::parseMesh()
{
    // 이전에 구성한 메시를 가지는 노드들을 순회.
    for (size_t nodeIndex = 0; nodeIndex < mFbxObjects.size(); ++nodeIndex)
    {
        VertexInfo vertexInfo;

        FbxMesh* mesh = mFbxObjects[nodeIndex].current->GetMesh();

        // 메시가 가지는 모든 정점들의 위치 배열
        FbxVector4* allVertexPos = mesh->GetControlPoints();
        // 메시가 가지는 폴리곤 수.
        size_t numPoly = mesh->GetPolygonCount();
        for(size_t polyIndex = 0; polyIndex < numPoly; ++polyIndex)
        {
            // 폴리곤의 구성단위 수(삼각형 || 사각형)
            size_t numVertexInPoly = mesh->GetPolygonSize(polyIndex);
            // 
            // 폴리곤의 구성은 삼각형뿐만 아니라 사각형 등과 같은 형태도 될 수 있음.
            // 따라서 사각형인 경우(==삼각형 두개)도 대응할 수 있도록 한다.
            // 삼각형으로 분해한다고도 볼 수 있을 듯.
            size_t numFaceInPoly = numVertexInPoly - 2;

            for(size_t faceIndex = 0; faceIndex < numFaceInPoly; ++faceIndex)
            {
                // 이 부분. 참고 자료에는 max와 dx는 서로 다른 방향으로 인덱스가 배치된다 했는데,
                // 현재 가지고 있는 fbx파일은 어느 모델링 툴을 사용했는지 불분명하기 때문에 특정시켜 구현 불가.
                // 하지만 일단 구현은 참고자료와 동일하게 했다.
                // 만약에 파일에 좌표계의 형식이 기록되어 있고, sdk가 읽을 수 있다면 한번 해보는 것도 좋을 듯.
                size_t indexListOfTriangle[3] = { 0, faceIndex + 2, faceIndex + 1 };

                // indexListOfTriangle에 따른 삼각형 하나의 인덱스를 돌면서 정점의 필요한 정보들을 얻어온다.
                // 한번에 3개의 점 -> for문으로 하나씩
                int indexOfVertex;
                FbxVector4 normalIndex;
                for (size_t index = 0; index < 3; ++index)
                {
                    indexOfVertex = mesh->GetPolygonVertex(polyIndex, indexListOfTriangle[index]);
                    mesh->GetPolygonVertexNormal(polyIndex, indexListOfTriangle[index], normalIndex);

                    FbxVector4 position = allVertexPos[indexOfVertex].mData;
                    vertexInfo.pos.x = position[0];
                    vertexInfo.pos.y = position[2];
                    vertexInfo.pos.z = position[1];

                    FbxVector4 normal = normalIndex.mData;
                    vertexInfo.norm.x = normal[0];
                    vertexInfo.norm.y = normal[2];
                    vertexInfo.norm.z = normal[1];

                    double* uv = nullptr;
                    // 텍스처를 가진 메시만
                    vertexInfo.tex.x = 0.0f;
                    vertexInfo.tex.y = 0.0f;
                    if(mesh->GetElementUVCount() > 0)
                    {
                        FbxGeometryElementUV* texture = mesh->GetElementUV(0);
                        if(texture->GetMappingMode() == FbxGeometryElement::eByControlPoint
                        && texture->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        {
                            size_t indexOfUV = texture->GetIndexArray().GetAt(indexOfVertex);
                            uv = texture->GetDirectArray().GetAt(indexOfVertex).mData;
                        }
                        else
                        {
                            uv = texture->GetDirectArray().GetAt(indexOfVertex).mData;
                        }

                        vertexInfo.tex.x = uv[0];
                        vertexInfo.tex.y = 1.0f - uv[1];
                    }


                    mFbxVertexInfo.push_back(vertexInfo);
                
                }

            }
        }

    }
}

/*
 *  sdk sample code
 */
void ModelImporter::printNode(FbxNode* node)
{

    const char* nodeName = node->GetName();
    FbxDouble3 translation = node->LclTranslation.Get();
    FbxDouble3 rotation = node->LclRotation.Get();
    FbxDouble3 scaling = node->LclScaling.Get();

    char str[256];
    sprintf_s(str, "<node name='%s' translation='(%lf, %lf, %lf)' rotation='(%lf, %lf, %lf)' scaling='(%lf, %lf, %lf)'>\n",
        nodeName,
        translation[0], translation[1], translation[2],
        rotation[0], rotation[1], rotation[2],
        scaling[0], scaling[1], scaling[2]);

    OutputDebugStringA(str);

    // Print the node's attributes.
    for (int i = 0; i < node->GetNodeAttributeCount(); i++)
        printAttribute(node->GetNodeAttributeByIndex(i));

    // Recursively print the children.
    for (int j = 0; j < node->GetChildCount(); j++)
        printNode(node->GetChild(j));

    OutputDebugStringA("</node>\n");
}

/**
 *
 *  sdk sample code
 *
* Return a string-based representation based on the attribute type.
*/
FbxString ModelImporter::getAttributeTypeName(FbxNodeAttribute::EType type) {
    switch (type) {
    case FbxNodeAttribute::eUnknown: return "unidentified";
    case FbxNodeAttribute::eNull: return "null";
    case FbxNodeAttribute::eMarker: return "marker";
    case FbxNodeAttribute::eSkeleton: return "skeleton";
    case FbxNodeAttribute::eMesh: return "mesh";
    case FbxNodeAttribute::eNurbs: return "nurbs";
    case FbxNodeAttribute::ePatch: return "patch";
    case FbxNodeAttribute::eCamera: return "camera";
    case FbxNodeAttribute::eCameraStereo: return "stereo";
    case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
    case FbxNodeAttribute::eLight: return "light";
    case FbxNodeAttribute::eOpticalReference: return "optical reference";
    case FbxNodeAttribute::eOpticalMarker: return "marker";
    case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
    case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
    case FbxNodeAttribute::eBoundary: return "boundary";
    case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
    case FbxNodeAttribute::eShape: return "shape";
    case FbxNodeAttribute::eLODGroup: return "lodgroup";
    case FbxNodeAttribute::eSubDiv: return "subdiv";
    default: return "unknown";
    }
}

/**
 *
 *  sdk sample code
 * Print an attribute.
 */
void ModelImporter::printAttribute(FbxNodeAttribute* pAttribute) {
    if (!pAttribute) return;

    FbxString typeName = getAttributeTypeName(pAttribute->GetAttributeType());
    FbxString attrName = pAttribute->GetName();

    // Note: to retrieve the character array of a FbxString, use its Buffer() method.
    char str[256];
    sprintf_s(str, "\n<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
    OutputDebugStringA(str);

}

// /*
//  *  vertex position
//  */
//  // controlPoint == vertex == polygon vertex(?)
// // 정점에 대한 유용한 정보는 위치 정보만 존재함.
// mFbxVertexCount = mesh->GetControlPointsCount();
// FbxVector4 vertex;
// for (size_t i = 0; i < mFbxVertexCount; ++i)
// {
//     vertex = mesh->GetControlPointAt(i);
//     // 좌표계가 다를텐데 괜찮은가?
//     // 조사 필요.
///*     mFbxPositionList.at(i).x = vertex.mData[0];
//     mFbxPositionList.at(i).y = vertex.mData[1];
//     mFbxPositionList.at(i).z = vertex.mData[2];*/
//     mFbxPositionList.push_back(XMFLOAT3(vertex.mData[0], vertex.mData[1], vertex.mData[2]));
// }

// // normal, uv 등 정보는 mesh의 surface 에서
// size_t polygonCount = mesh->GetPolygonCount();
// for (size_t poly = 0; poly < polygonCount; ++poly)
// {
//     // 한 폴리곤의 버텍스 순회
//     for(size_t indexInPoly = 0; indexInPoly < 3; ++indexInPoly)
//     {
//         // 폴리곤을 구성하는 버텍스의 인덱스를 반환. == 인덱스 리스트를 구성을 여기에서.
//         // 인덱스 리스트를 이용하여 버텍스 중복을 해결하긴 하지만, 렌더링시에 삼각형 단위로 그리는 반면,
//         // fbxsdk에서는 이런 개념이 아니라고 한다. 
//         size_t indexOfVertex = mesh->GetPolygonVertex(poly, indexInPoly);
//         // 그렇다면 이것도 좌표계를 변환해야 할 텐데.
//         // 조사 필요.
//         mFbxIndexList.push_back(indexOfVertex);

//         /*
//          *  normal
//          */
//         FbxGeometryElementNormal* normalInfo = mesh->GetElementNormal();
//         ASSERT(mesh->GetElementNormalCount() > 1, "normal 데이터가 존재하지 않습니다.");
//         
//         // 게임엔진에 대해 주요 타입은 아래 두 타입 정도가 신경 쓸 부분이라 한다.
//         // FbxGeometryElement::eByControlPoint, FbxGeometryElement::eByPolygonVertex
//         // eByControlPoint 는 정점이 노말을 하나만 가지는 경우.
//         // eByPolygonVertex는 한 정점이 여러 면에서 다른 노말을 가지는 경우.
//         // 큐브의 꼭짓점 같은 경우를 말하는 듯.
//         // Polygon으로 나뉜 이유가 이런식으로 한 정점이 여러 노말을 가질 수 있도록 하는 방식이 될 수 있을 듯.
//         switch(normalInfo->GetMappingMode())
//         {
//             case FbxGeometryElement::eByControlPoint:
//             {
//                 if (normalInfo->GetReferenceMode() == FbxGeometryElement::eDirect)
//                 {
//                     FbxVector4 normal = normalInfo->GetDirectArray().GetAt(indexOfVertex);
//                     //mFbxNormalList.at(indexOfVertex).x = normal.mData[0];
//                     //mFbxNormalList.at(indexOfVertex).y = normal.mData[1];
//                     //mFbxNormalList.at(indexOfVertex).z = normal.mData[2];
//                     mFbxNormalList.push_back(XMFLOAT3(normal.mData[0], normal.mData[1], normal.mData[2]));


//                 }
//                 else if (normalInfo->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
//                 {
//                     // 위에서 말한 이유로 이 오브젝트의 노말의 매핑 모드에 따라 어떤 배치를 따른다고 볼 수 있겠다.
//                     // eDirect 인덱스와 버텍스 순서가 같은 경우.
//                     // eIndexToDirect 중복되는 버텍스가 속하는 면마다 다른 노말을 가져서 인덱스와 버텍스 순서가 안맞는 경우.

//                     int index = normalInfo->GetIndexArray().GetAt(indexOfVertex);
//                     FbxVector4 normal = normalInfo->GetDirectArray().GetAt(index);
///*                     mFbxNormalList.at(indexOfVertex).x = normal.mData[0];
//                     mFbxNormalList.at(indexOfVertex).y = normal.mData[1];
//                     mFbxNormalList.at(indexOfVertex).z = normal.mData[2];*/
//                     mFbxNormalList.push_back(XMFLOAT3(normal.mData[0], normal.mData[1], normal.mData[2]));

//                 }

//                 break;
//             }
//             case FbxGeometryElement::eByPolygonVertex:
//             {
//                 // 아직 좀 더 생각해보거나 조사하기
//                 // : 인덱스 리스트를 사용하지 않고 그린다면, 중복되는 버텍스 정보들을 가진다는 건 알겠는데,
//                 // : uv 같은 건? ==> 문제 없을 듯?
//                 ASSERT(false, "아직 이해 안가는 부분");
//                 int count = 0; // 0부터 vertexCount까지 하나하나 정보를 채워주는 인덱스
//                 if (normalInfo->GetReferenceMode() == FbxGeometryElement::eDirect)
//                 {
//                     FbxVector4 normal = normalInfo->GetDirectArray().GetAt(count);
//                     mFbxNormalList.at(indexOfVertex).x = normal.mData[0];
//                     mFbxNormalList.at(indexOfVertex).y = normal.mData[1];
//                     mFbxNormalList.at(indexOfVertex).z = normal.mData[2];
//                 }
//                 else if (normalInfo->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
//                 {
//                     // 위에서 말한 이유로 이 오브젝트의 노말의 매핑 모드에 따라 어떤 배치를 따른다고 볼 수 있겠다.
//                     // eDirect 인덱스와 버텍스 순서가 같은 경우.
//                     // eIndexToDirect 중복되는 버텍스가 속하는 면마다 다른 노말을 가져서 인덱스와 버텍스 순서가 안맞는 경우.

//                     int index = normalInfo->GetIndexArray().GetAt(count);
//                     FbxVector4 normal = normalInfo->GetDirectArray().GetAt(index);
//                     mFbxNormalList.push_back(XMFLOAT3(normal.mData[0], normal.mData[1], normal.mData[2]));
//                 }
//                 break;
//             }
//             default:
//                 OutputDebugStringA("ModelImporter::printMesh-normal : 지원하지 않는 타입입니다.");
//                 break;
//         }


//         /*
//          *  uv
//          */
//         ASSERT(mesh->GetElementUVCount() >= 1, "normal 데이터가 존재하지 않습니다.");
//         // 모델이 여러 텍스처를 가질수도 있다.
//         // 첫번째것만 씀 == 0
//         FbxGeometryElementUV* texture = mesh->GetElementUV(0);
//         
//         switch(texture->GetMappingMode())
//         {
//             case FbxGeometryElement::eByControlPoint:
//             {
//                 if(texture->GetReferenceMode() == FbxGeometryElement::eDirect)
//                 {
//                     mFbxTexUVList.at(indexOfVertex).x = texture->GetDirectArray().GetAt(indexOfVertex).mData[0];
//                     mFbxTexUVList.at(indexOfVertex).y = texture->GetDirectArray().GetAt(indexOfVertex).mData[1];
//                 }
//                 else if(texture->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
//                 {
//                     size_t index = texture->GetIndexArray().GetAt(indexOfVertex);
//                     mFbxTexUVList.at(indexOfVertex).x = texture->GetDirectArray().GetAt(index).mData[0];
//                     mFbxTexUVList.at(indexOfVertex).y = texture->GetDirectArray().GetAt(index).mData[0];
//                 }
//                 else
//                 {
//                     OutputDebugStringA("ModelImporter::printMesh-uv-eByControlPoint : 지원하지 않는 타입입니다.");
//                 }
//                 break;
//             }
//             case FbxGeometryElement::eByPolygonVertex:
//             {
//                 if(texture->GetReferenceMode() == FbxGeometryElement::eIndexToDirect || texture->GetReferenceMode()== FbxGeometryElement::eDirect)
//                 {
//                     int count = 0; // 0부터 vertexCount까지 하나하나 정보를 채워주는 인덱스

//                     mFbxTexUVList.at(indexOfVertex).x = texture->GetDirectArray().GetAt(count).mData[0];
//                     mFbxTexUVList.at(indexOfVertex).y = texture->GetDirectArray().GetAt(count).mData[1];
//                 }
//                 else
//                 {
//                     OutputDebugStringA("ModelImporter::printMesh-uv-eByPolygonVertex : 지원하지 않는 타입입니다.");
//                 }
//                 break;
//             }
//             default:
//                 OutputDebugStringA("ModelImporter::printMesh-uv : 지원하지 않는 타입입니다.");
//                 break;
//         }

//         

//     }
// }
