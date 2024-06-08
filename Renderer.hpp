#if !defined(DX12_RASTERIZER_H)

#include <d3d12.h>
#include <dxgi1_3.h>
#include "assets.h"
#include <vector>

struct dir_light_buffer_cpu
{
    v3 LightColor;
    f32 LightAmbientIntensity;
    v3 LightDirection;
    v3 CameraPos;
};

struct point_light_cpu
{
    v3 Pos;
    f32 DivisorConstant;
    v3 Color;
};

struct transform_buffer_cpu
{
    m4 WVPTransform;
    m4 WTransform;
    m4 NormalWTransform;
    f32 Shininess;
    f32 SpecularStrength;
};

struct dx12_arena
{
    u64 Size;
    u64 Used;
    ID3D12Heap* Heap;
};

struct dx12_upload_arena
{
    u64 Size;
    u64 Used;
    ID3D12Resource* GpuBuffer;
    u8* CpuPtr;
};

class dx12_descriptor_heap
{
public:
    ID3D12DescriptorHeap* Heap;
    int StepSize;
    int MaxNumElements;
    int CurrElement;

    void init_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE Type, u64 NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS Flags);
    void allocate(D3D12_CPU_DESCRIPTOR_HANDLE* OutCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* OutGpuHandle);
};

struct asset
{
    ID3D12Resource* TransformBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE TransformDescriptor;
    model Model;
};

class Renderer
{
public:
    IDXGIAdapter1* Adapter;
    ID3D12Device* Device;
    ID3D12CommandQueue* CommandQueue;

    dx12_upload_arena UploadArena;
    dx12_arena RtvArena;
    dx12_arena BufferArena;
    dx12_arena TextureArena;

    u32 RenderWidth;
    u32 RenderHeight;
    IDXGISwapChain1* SwapChain;
    u32 CurrentFrameIndex;
    ID3D12Resource* FrameBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE FrameBufferDescriptors[2];

    D3D12_CPU_DESCRIPTOR_HANDLE DepthDescriptor;
    ID3D12Resource* DepthBuffer;
    
    ID3D12CommandAllocator* CommandAllocator;

    ID3D12GraphicsCommandList* CommandList;
    ID3D12Fence* Fence;
    UINT64 FenceValue;

    asset cube;
    asset sponza;
    asset sphere;
    std::vector<asset> object;

    dx12_descriptor_heap RtvHeap;
    dx12_descriptor_heap DsvHeap;
    dx12_descriptor_heap ShaderDescHeap;

#define NUM_POINT_LIGHTS 2

    ID3D12Resource* DirLightBuffer;
    ID3D12Resource* PointLightBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE LightDescriptor;

    ID3D12RootSignature* ModelRootSignature;
    ID3D12PipelineState* ModelPipeline;

    // API CALLS
    void clear_render_view(const FLOAT color[4]);
    void upload_transform_buffer(ID3D12Resource* Resource, m4 WTransform, m4 VPTransform, f32 Shininess, f32 SpecularStrength);
    void execute(bool is_first);
    void set_viewport();
    void set_barrier(D3D12_RESOURCE_STATES start_state, D3D12_RESOURCE_STATES end_state, ID3D12Resource* resource);
    void render_model(asset* Model);
    void draw_objects();

    void Dx12CopyDataToBuffer(D3D12_RESOURCE_STATES StartState, D3D12_RESOURCE_STATES EndState, void* Data, u64 DataSize, ID3D12Resource* GpuBuffer);
    void Dx12CreateTexture(u32 Width, u32 Height, u8* Texels, ID3D12Resource** OutResource, D3D12_GPU_DESCRIPTOR_HANDLE* OutDescriptor);
    ID3D12Resource* Dx12CreateBufferAsset(D3D12_RESOURCE_DESC* Desc, D3D12_RESOURCE_STATES InitialState, void* BufferData);
    ID3D12Resource* Dx12CreateBufferAsset(u32 Size, D3D12_RESOURCE_STATES State, void* Data);
    void Dx12CreateConstantBuffer(u32 Size, ID3D12Resource** OutResource, D3D12_GPU_DESCRIPTOR_HANDLE* OutDescriptor);
    Renderer(HWND WindowHandle, u32 Width, u32 Height);
private:
    dx12_upload_arena Dx12UploadArenaCreate(u64 Size);
    
    dx12_arena Dx12ArenaCreate(D3D12_HEAP_TYPE Type, u64 Size, D3D12_HEAP_FLAGS Flags);
    u8* Dx12UploadArenaPushSize(dx12_upload_arena* Arena, u64 Size, u64* OutOffset);
    ID3D12Resource* Dx12CreateResource(dx12_arena* Arena, D3D12_RESOURCE_DESC* Desc, D3D12_RESOURCE_STATES InitialState, D3D12_CLEAR_VALUE* ClearValues);
    D3D12_SHADER_BYTECODE Dx12LoadShader(const char* FileName);
    ID3D12Resource* Dx12CreateTextureAsset(D3D12_RESOURCE_DESC* Desc, D3D12_RESOURCE_STATES InitialState, void* Texels);
    void Dx12ClearUploadArena(dx12_upload_arena* Arena);    
};

#define DX12_RASTERIZER_H
#endif
