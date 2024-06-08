#if !defined(ASSETS_H)

#undef global
#undef local_global

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define global static
#define local_global static

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma pack(push, 1)
struct vertex
{
    v3 Pos;
    v2 Uv;
    v3 Normal;
};

struct texel_rgba8
{
    u8 Red;
    u8 Green;
    u8 Blue;
    u8 Alpha;
};
#pragma pack(pop)

struct texture
{
    int Width;
    int Height;
    int* Texels;

    ID3D12Resource* GpuTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE GpuDescriptor;
};

struct mesh
{
    int IndexOffset;
    int IndexCount;
    int VertexOffset;
    int VertexCount;
    int TextureId;
};

struct model
{
    int NumMeshes;
    mesh* MeshArray;

    int NumTextures;
    texture* TextureArray;
    
    int VertexCount;
    vertex* VertexArray;
    int IndexCount;
    int* IndexArray;

    ID3D12Resource* GpuVertexBuffer;
    ID3D12Resource* GpuIndexBuffer;

    ID3D12Resource* TransformBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE TransformDescriptor;
};

#define ASSETS_H
#endif
