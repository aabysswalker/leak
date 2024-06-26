#if !defined(DX12_RASTERIZER_H)



struct dir_light_buffer_cpu
{
    dxm::XMFLOAT3 LightColor;
    f32 LightAmbientIntensity;
    dxm::XMFLOAT3 LightDirection;
    u32 NumPointLights;
    dxm::XMFLOAT3 CameraPos;
};

struct asset
{
    model model;
    ID3D12Resource* TransformBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE TransformDescriptor;
};

struct point_light_cpu
{
    dxm::XMFLOAT3 Pos;
    f32 DivisorConstant;
    dxm::XMFLOAT3 Color;
};

struct transform_buffer_cpu
{
    dxm::XMMATRIX WVPTransform;
    dxm::XMMATRIX WTransform;
    dxm::XMMATRIX NormalWTransform;
    f32 Shininess;
    f32 SpecularStrength;
    bool is_pbr;
};

struct dx12_upload_arena
{
    u64 Size;
    u64 Used;
    ID3D12Resource* GpuBuffer;
    u8* CpuPtr;
};

struct dx12_descriptor_heap
{
    ID3D12DescriptorHeap* Heap;
    u64 StepSize;
    u64 MaxNumElements;
    u64 CurrElement;
};

struct dx12_rasterizer
{
    IDXGIAdapter1* Adapter;
    ID3D12Device* Device;
    ID3D12CommandQueue* CommandQueue;

    dx12_upload_arena UploadArena;

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

    std::vector<asset> objects;

    dx12_descriptor_heap RtvHeap;
    dx12_descriptor_heap DsvHeap;
    dx12_descriptor_heap ShaderDescHeap;


    // NOTE: Дані пов'язані з світлом
#define NUM_POINT_LIGHTS 2

    ID3D12Resource* DirLightBuffer;
    ID3D12Resource* PointLightBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE LightDescriptor;
    
    // NOTE: Графічний конвеєр
    ID3D12RootSignature* ModelRootSignature;
    ID3D12PipelineState* ModelPipeline;
};

#define DX12_RASTERIZER_H
#endif
