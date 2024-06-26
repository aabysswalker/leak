#include <vector>

char* CombineStrings(const char* Str1, const char* Str2)
{
    char* Result = 0;
    mm Length1 = strlen(Str1);
    mm Length2 = strlen(Str2);
    Result = (char*)malloc(sizeof(char) * (Length1 + Length2 + 1));
    memcpy(Result, Str1, Length1 * sizeof(char));
    memcpy(Result + Length1, Str2, Length2 * sizeof(char));
    Result[Length1 + Length2] = 0;

    return Result;
}
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923 // pi/2
#endif

texture AssetCreateDefaultNormalMap()
{
    local_global uint32_t NormalTexel = 0xFF8080FF;

    texture Result = {};
    Result.Width = 1;
    Result.Height = 1;
    Result.Texels = &NormalTexel;

    return Result;
}

void LoadTexture(dx12_rasterizer* Dx12Rasterizer, aiTextureType type, aiMaterial* CurrMaterial, texture* Texture, int CurrTextureId, std::string file_path)
{
    {
        aiString TextureName = {};
        CurrMaterial->GetTexture(type, 0, &TextureName);

        texture* CurrTexture = Texture + CurrTextureId;

        std::string TexturePath = file_path + TextureName.data;

        {
            OutputDebugStringA(TexturePath.c_str());
            OutputDebugStringA("\n");
        }

        i32 NumChannels = 0;
        u32* UnFlippedTexels = (u32*)stbi_load(TexturePath.c_str(), &CurrTexture->Width, &CurrTexture->Height, &NumChannels, 4);

        CurrTexture->Texels = (u32*)malloc(sizeof(u32) * CurrTexture->Width * CurrTexture->Height);

        for (u32 Y = 0; Y < CurrTexture->Height; ++Y)
        {
            for (u32 X = 0; X < CurrTexture->Width; ++X)
            {
                u32 PixelId = Y * CurrTexture->Width + X;
                CurrTexture->Texels[PixelId] = UnFlippedTexels[(CurrTexture->Height - Y - 1) * CurrTexture->Width + X];
            }
        }

        Dx12CreateTexture(Dx12Rasterizer, CurrTexture->Width, CurrTexture->Height, (u8*)CurrTexture->Texels,
            &CurrTexture->GpuTexture, &CurrTexture->GpuDescriptor);

        stbi_image_free(UnFlippedTexels);
    }
}


model AssetLoadModel(dx12_rasterizer* Dx12Rasterizer, std::string FilePath, std::string FileName, dxm::XMMATRIX transform, bool is_pbr = false)
{
    model Result = {};
    Result.transform = transform;
    Result.is_pbr = is_pbr;
    asset dummy;
    Dx12Rasterizer->objects.push_back(dummy);
    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(FilePath + FileName, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_OptimizeMeshes);
    if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
    {
        const char* Error = Importer.GetErrorString();
        InvalidCodePath;
    }
    
    u32* TextureMappingTable = (u32*)malloc(sizeof(u32) * Scene->mNumMaterials);
    for (u32 MaterialId = 0; MaterialId < Scene->mNumMaterials; ++MaterialId)
    {

        aiMaterial* CurrMaterial = Scene->mMaterials[MaterialId]; 

        if (CurrMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            TextureMappingTable[MaterialId] = Result.NumTextures;
            Result.NumTextures += 1;
        }
        else
        {
            TextureMappingTable[MaterialId] = 0xFFFFFFFF;
        }
    }

    Result.ColorTextureArray = (texture*)malloc(sizeof(texture) * Result.NumTextures);
    Result.NormalTextureArray = (texture*)malloc(sizeof(texture) * Result.NumTextures);
    Result.MRTextureArray = (texture*)malloc(sizeof(texture) * Result.NumTextures);

    u32 CurrTextureId = 0;
    for (u32 MaterialId = 0; MaterialId < Scene->mNumMaterials; ++MaterialId)
    {
        aiMaterial* CurrMaterial = Scene->mMaterials[MaterialId];

        if (CurrMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            LoadTexture(Dx12Rasterizer, aiTextureType_DIFFUSE, CurrMaterial, Result.ColorTextureArray, CurrTextureId, FilePath);
            
            {
                aiString TextureName = {};
                CurrMaterial->GetTexture(aiTextureType_NORMALS, 0, &TextureName);

                texture* CurrTexture = Result.NormalTextureArray + CurrTextureId;

                if (CurrMaterial->GetTextureCount(aiTextureType_NORMALS) == 0)
                {
                    *CurrTexture = AssetCreateDefaultNormalMap();

                    Dx12CreateTexture(Dx12Rasterizer, CurrTexture->Width, CurrTexture->Height, (u8*)CurrTexture->Texels,
                        &CurrTexture->GpuTexture, &CurrTexture->GpuDescriptor, true);
                }
                else
                {
                    char* TexturePath = CombineStrings(FilePath.c_str(), TextureName.C_Str());

                    {
                        OutputDebugStringA(TexturePath);
                        OutputDebugStringA("\n");
                    }

                    i32 NumChannels = 0;
                    u32* UnFlippedTexels = (u32*)stbi_load(TexturePath, &CurrTexture->Width, &CurrTexture->Height, &NumChannels, 4);

                    CurrTexture->Texels = (u32*)malloc(sizeof(u32) * CurrTexture->Width * CurrTexture->Height);

                    for (u32 Y = 0; Y < CurrTexture->Height; ++Y)
                    {
                        for (u32 X = 0; X < CurrTexture->Width; ++X)
                        {
                            u32 PixelId = Y * CurrTexture->Width + X;
                            CurrTexture->Texels[PixelId] = UnFlippedTexels[(CurrTexture->Height - Y - 1) * CurrTexture->Width + X];
                        }
                    }

                    Dx12CreateTexture(Dx12Rasterizer, CurrTexture->Width, CurrTexture->Height, (u8*)CurrTexture->Texels,
                        &CurrTexture->GpuTexture, &CurrTexture->GpuDescriptor, false);

                    stbi_image_free(UnFlippedTexels);
                }
            }
            if (CurrMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) != 0)
            {
                LoadTexture(Dx12Rasterizer, aiTextureType_DIFFUSE_ROUGHNESS, CurrMaterial, Result.MRTextureArray, CurrTextureId, FilePath);
            }

            CurrTextureId += 1;
        }
    }
    
    Result.NumMeshes = Scene->mNumMeshes;
    Result.MeshArray = (mesh*)malloc(sizeof(mesh) * Result.NumMeshes);
    for (u32 MeshId = 0; MeshId < Result.NumMeshes; ++MeshId)
    {
        aiMesh* SrcMesh = Scene->mMeshes[MeshId];
        mesh* DstMesh = Result.MeshArray + MeshId;

        DstMesh->TextureId = TextureMappingTable[SrcMesh->mMaterialIndex];
        DstMesh->IndexOffset = Result.IndexCount;
        DstMesh->VertexOffset = Result.VertexCount;
        DstMesh->IndexCount = SrcMesh->mNumFaces * 3;
        DstMesh->VertexCount = SrcMesh->mNumVertices;

        Result.IndexCount += DstMesh->IndexCount;
        Result.VertexCount += DstMesh->VertexCount;
    }

    Result.VertexArray = (vertex*)malloc(sizeof(vertex) * Result.VertexCount);
    Result.IndexArray = (u32*)malloc(sizeof(u32) * Result.IndexCount);

    f32 MinDistAxis = FLT_MAX;
    f32 MaxDistAxis = FLT_MIN;
    for (u32 MeshId = 0; MeshId < Result.NumMeshes; ++MeshId)
    {
        aiMesh* SrcMesh = Scene->mMeshes[MeshId];
        mesh* DstMesh = Result.MeshArray + MeshId;

        for (u32 VertexId = 0; VertexId < DstMesh->VertexCount; ++VertexId)
        {
            vertex* CurrVertex = Result.VertexArray + DstMesh->VertexOffset + VertexId;
            CurrVertex->Pos = dxm::XMFLOAT3(SrcMesh->mVertices[VertexId].x,
                                 SrcMesh->mVertices[VertexId].y,
                                 SrcMesh->mVertices[VertexId].z);

            MinDistAxis = std::min(MinDistAxis, std::min(CurrVertex->Pos.x, std::min(CurrVertex->Pos.y, CurrVertex->Pos.z)));
            MaxDistAxis = max(MaxDistAxis, max(CurrVertex->Pos.x, max(CurrVertex->Pos.y, CurrVertex->Pos.z)));

            CurrVertex->Normal = dxm::XMFLOAT3(SrcMesh->mNormals[VertexId].x,
                                    SrcMesh->mNormals[VertexId].y,
                                    SrcMesh->mNormals[VertexId].z);

            CurrVertex->Tangent = dxm::XMFLOAT3(SrcMesh->mTangents[VertexId].x,
                SrcMesh->mTangents[VertexId].y,
                SrcMesh->mTangents[VertexId].z);

            CurrVertex->BiTangent = dxm::XMFLOAT3(SrcMesh->mBitangents[VertexId].x,
                SrcMesh->mBitangents[VertexId].y,
                SrcMesh->mBitangents[VertexId].z);
            
            if (SrcMesh->mTextureCoords[0])
            {
                CurrVertex->Uv = dxm::XMFLOAT2(SrcMesh->mTextureCoords[0][VertexId].x,
                                    SrcMesh->mTextureCoords[0][VertexId].y);
            }
            else
            {
                CurrVertex->Uv = dxm::XMFLOAT2(0, 0);
            }
        }

        for (u32 FaceId = 0; FaceId < SrcMesh->mNumFaces; ++FaceId)
        {
            u32* CurrIndices = Result.IndexArray + DstMesh->IndexOffset + FaceId * 3;

            CurrIndices[0] = SrcMesh->mFaces[FaceId].mIndices[0];
            CurrIndices[1] = SrcMesh->mFaces[FaceId].mIndices[1];
            CurrIndices[2] = SrcMesh->mFaces[FaceId].mIndices[2];
        }
    }
    
    for (u32 VertexId = 0; VertexId < Result.VertexCount; ++VertexId)
    {
        vertex* CurrVertex = Result.VertexArray + VertexId;

        dxm::XMFLOAT3 minDistAxis(MinDistAxis, MinDistAxis, MinDistAxis);
        dxm::XMFLOAT3 maxDistAxis(MaxDistAxis, MaxDistAxis, MaxDistAxis);

        dxm::XMFLOAT3 adjustment(0.5f, 0.5f, 0.5f);

        dxm::XMFLOAT3 temp;
        XMStoreFloat3(&temp, dxm::XMVectorSubtract(XMLoadFloat3(&CurrVertex->Pos), XMLoadFloat3(&minDistAxis)));
        XMStoreFloat3(&temp, dxm::XMVectorDivide(XMLoadFloat3(&temp), dxm::XMVectorSubtract(XMLoadFloat3(&maxDistAxis), XMLoadFloat3(&minDistAxis))));

        XMStoreFloat3(&temp, dxm::XMVectorSubtract(XMLoadFloat3(&temp), XMLoadFloat3(&adjustment)));

        CurrVertex->Pos = temp;
    }

    Result.GpuVertexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.VertexCount * sizeof(vertex), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, Result.VertexArray);
    Result.GpuIndexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.IndexCount * sizeof(u32), D3D12_RESOURCE_STATE_INDEX_BUFFER, Result.IndexArray);

    return Result;
}
