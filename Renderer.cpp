#include <memory>

void dx12_descriptor_heap::init_heap(ID3D12Device* device ,D3D12_DESCRIPTOR_HEAP_TYPE Type, u64 NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
{
    MaxNumElements = NumDescriptors;
    StepSize = device->GetDescriptorHandleIncrementSize(Type);

    D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
    Desc.Type = Type;
    Desc.NumDescriptors = NumDescriptors;
    Desc.Flags = Flags;
    device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Heap));
}

void dx12_descriptor_heap::allocate(D3D12_CPU_DESCRIPTOR_HANDLE* OutCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* OutGpuHandle)
{
    Assert(CurrElement < MaxNumElements);

    if (OutCpuHandle)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = Heap->GetCPUDescriptorHandleForHeapStart();
        CpuHandle.ptr += StepSize * CurrElement;
        *OutCpuHandle = CpuHandle;
    }

    if (OutGpuHandle)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle = Heap->GetGPUDescriptorHandleForHeapStart();
        GpuHandle.ptr += StepSize * CurrElement;
        *OutGpuHandle = GpuHandle;
    }

    CurrElement += 1;
}

void Renderer::set_viewport()
{
    D3D12_VIEWPORT Viewport = {};
    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width = RenderWidth;
    Viewport.Height = RenderHeight;
    Viewport.MinDepth = 0;
    Viewport.MaxDepth = 1;
    CommandList->RSSetViewports(1, &Viewport);

    D3D12_RECT ScissorRect = {};
    ScissorRect.left = 0;
    ScissorRect.right = RenderWidth;
    ScissorRect.top = 0;
    ScissorRect.bottom = RenderHeight;
    CommandList->RSSetScissorRects(1, &ScissorRect);

    CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

void Renderer::execute(bool is_first)
{
    CommandList->Close();
    CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&CommandList);
    if (!is_first)
    {
        SwapChain->Present(1, 0);
    }
 
    FenceValue += 1;
    CommandQueue->Signal(Fence, FenceValue);
    if (Fence->GetCompletedValue() != FenceValue)
    {
        HANDLE FenceEvent = {};
        Fence->SetEventOnCompletion(FenceValue, FenceEvent);
        WaitForSingleObject(FenceEvent, INFINITE);
    }

    CommandAllocator->Reset();
    CommandList->Reset(CommandAllocator, 0);

    
    Dx12ClearUploadArena(&UploadArena);   
}

void Renderer::clear_render_view(const FLOAT color[4])
{
    CommandList->ClearRenderTargetView(FrameBufferDescriptors[CurrentFrameIndex], color, 0, nullptr);
    CommandList->ClearDepthStencilView(DepthDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, 0);
    CommandList->OMSetRenderTargets(1, FrameBufferDescriptors + CurrentFrameIndex, 0, &DepthDescriptor);
}

void Renderer::set_barrier(D3D12_RESOURCE_STATES start_state, D3D12_RESOURCE_STATES end_state, ID3D12Resource* resource)
{
    D3D12_RESOURCE_BARRIER Barrier = {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Transition.pResource = resource;
    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    Barrier.Transition.StateBefore = start_state;
    Barrier.Transition.StateAfter = end_state;
    CommandList->ResourceBarrier(1, &Barrier);
}


void ThrowIfFailed(HRESULT Result)
{
    if (Result != S_OK)
    {
        InvalidCodePath;
    }
}

u32 Dx12GetBytesPerPixel(DXGI_FORMAT Format)
{
    u32 Result = 0;
    
    switch (Format)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            Result = 16;
        } break;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        {
            Result = 8;
        } break;
        
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        {
            Result = 4;
        } break;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        {
            Result = 2;
        } break;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        {
            Result = 1;
        } break;
    }

    return Result;
}

dx12_arena Renderer::Dx12ArenaCreate(D3D12_HEAP_TYPE Type, u64 Size, D3D12_HEAP_FLAGS Flags)
{
    dx12_arena Result = {};
    Result.Size = Size;

    D3D12_HEAP_DESC Desc = {};
    Desc.SizeInBytes = Size;
    Desc.Properties.Type = Type;
    Desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    Desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    Desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    Desc.Flags = Flags;

    ThrowIfFailed(Device->CreateHeap(&Desc, IID_PPV_ARGS(&Result.Heap)));
    
    return Result;
}

ID3D12Resource* Renderer::Dx12CreateResource(dx12_arena* Arena, D3D12_RESOURCE_DESC* Desc, D3D12_RESOURCE_STATES InitialState, D3D12_CLEAR_VALUE* ClearValues)
{
    ID3D12Resource* Result = 0;

    D3D12_RESOURCE_ALLOCATION_INFO AllocationInfo = Device->GetResourceAllocationInfo(0, 1, Desc);
    u64 GpuAlignedOffset = Align(Arena->Used, AllocationInfo.Alignment);
    Assert((GpuAlignedOffset + AllocationInfo.SizeInBytes) < Arena->Size);
    
    ThrowIfFailed(Device->CreatePlacedResource(Arena->Heap, GpuAlignedOffset,
                                               Desc, InitialState, ClearValues,
                                               IID_PPV_ARGS(&Result)));
    Arena->Used = GpuAlignedOffset + AllocationInfo.SizeInBytes;

    return Result;
}

dx12_upload_arena Renderer::Dx12UploadArenaCreate(u64 Size)
{
    dx12_upload_arena Result = {};
    Result.Size = Size;
            
    D3D12_HEAP_PROPERTIES HeapProperties = {};
    HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    Desc.Width = Size;
    Desc.Height = 1;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ThrowIfFailed(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &Desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  0, IID_PPV_ARGS(&Result.GpuBuffer)));
    Result.GpuBuffer->Map(0, 0, (void**)&Result.CpuPtr);

    return Result;
}

u8* Renderer::Dx12UploadArenaPushSize(dx12_upload_arena* Arena, u64 Size, u64* OutOffset)
{
    int AlignedOffset = Align(Arena->Used, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
    Assert((AlignedOffset + Size) < Arena->Size);
    
    u8* Result = Arena->CpuPtr + AlignedOffset;
    Arena->Used = AlignedOffset + Size;
    *OutOffset = AlignedOffset;
    
    return Result;
}

void Renderer::Dx12ClearUploadArena(dx12_upload_arena* Arena)
{
    Arena->Used = 0;
}

void Renderer::Dx12CopyDataToBuffer(D3D12_RESOURCE_STATES StartState, D3D12_RESOURCE_STATES EndState, void* Data, u64 DataSize, ID3D12Resource* GpuBuffer)
{

    u64 UploadOffset = 0;

    {
        u8* Dest = Dx12UploadArenaPushSize(&UploadArena, DataSize, &UploadOffset);
        memcpy(Dest, Data, DataSize);
    }

    set_barrier(StartState, D3D12_RESOURCE_STATE_COPY_DEST, GpuBuffer);

    CommandList->CopyBufferRegion(GpuBuffer, 0, UploadArena.GpuBuffer, UploadOffset, DataSize);

    set_barrier(D3D12_RESOURCE_STATE_COPY_DEST, EndState, GpuBuffer);
}

ID3D12Resource* Renderer::Dx12CreateBufferAsset(D3D12_RESOURCE_DESC* Desc, D3D12_RESOURCE_STATES InitialState, void* BufferData)
{
    ID3D12Resource* Result = 0;

    u64 UploadOffset = 0;
    {
        u8* Dest = Dx12UploadArenaPushSize(&UploadArena,
                                           Desc->Width, &UploadOffset);
        memcpy(Dest, BufferData, Desc->Width);
    }

    Result = Dx12CreateResource(&BufferArena, Desc,
                                D3D12_RESOURCE_STATE_COPY_DEST, 0);
    CommandList->CopyBufferRegion(Result, 0,
                                  UploadArena.GpuBuffer,
                                  UploadOffset, Desc->Width);

    set_barrier(D3D12_RESOURCE_STATE_COPY_DEST, InitialState, Result);
    
    return Result;
}

ID3D12Resource* Renderer::Dx12CreateBufferAsset(u32 Size, D3D12_RESOURCE_STATES State, void* Data)
{
    ID3D12Resource* Result = 0;
    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    Desc.Width = Size;
    Desc.Height = 1;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    Result = Dx12CreateBufferAsset(&Desc, State, Data);

    return Result;
}

void Renderer::Dx12CreateConstantBuffer(u32 Size, ID3D12Resource** OutResource,
                              D3D12_GPU_DESCRIPTOR_HANDLE* OutDescriptor)
{
    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    Desc.Width = Align(Size, 256);
    Desc.Height = 1;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    *OutResource = Dx12CreateResource(&BufferArena,
                                      &Desc,
                                      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                      0);

    D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc = {};
    CbvDesc.BufferLocation = OutResource[0]->GetGPUVirtualAddress();
    CbvDesc.SizeInBytes = Desc.Width;

    D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = {};
    ShaderDescHeap.allocate(&CpuDescriptor, OutDescriptor);
    Device->CreateConstantBufferView(&CbvDesc, CpuDescriptor);
}

ID3D12Resource* Renderer::Dx12CreateTextureAsset(D3D12_RESOURCE_DESC* Desc, D3D12_RESOURCE_STATES InitialState, void* Texels)
{
    constexpr u32 MAX_MIP_LEVELS = 32;
    ID3D12Resource* Result = nullptr;

    D3D12_RESOURCE_ALLOCATION_INFO AllocationInfo = Device->GetResourceAllocationInfo(0, 1, Desc);
    u64 UploadSize = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT MipFootPrints[MAX_MIP_LEVELS] = {};
    Device->GetCopyableFootprints(Desc, 0, Desc->MipLevels, 0, MipFootPrints, nullptr, nullptr, &UploadSize);

    u64 UploadOffset = 0;
    auto UploadTexels = static_cast<u8*>(Dx12UploadArenaPushSize(&UploadArena, UploadSize, &UploadOffset));

    u64 BytesPerPixel = Dx12GetBytesPerPixel(Desc->Format);
    std::unique_ptr<texel_rgba8[]> MipMemory = Desc->MipLevels > 1 ? std::make_unique<texel_rgba8[]>(UploadSize / sizeof(texel_rgba8)) : nullptr;

    {
        auto* CurrFootPrint = &MipFootPrints[0];

        CurrFootPrint->Offset += UploadOffset;
        for (u32 Y = 0; Y < Desc->Height; ++Y)
        {
            u8* Src = static_cast<u8*>(Texels) + (Y * CurrFootPrint->Footprint.Width) * BytesPerPixel;
            u8* Dest = UploadTexels + (Y * CurrFootPrint->Footprint.RowPitch);
            memcpy(Dest, Src, BytesPerPixel * CurrFootPrint->Footprint.Width);
        }
    }

    if (MipMemory)
    {
        texel_rgba8* SrcMipStart = MipMemory.get() - MipFootPrints[0].Footprint.Width * MipFootPrints[0].Footprint.Height;
        texel_rgba8* DstMipStart = MipMemory.get();

        for (u32 MipId = 1; MipId < Desc->MipLevels; ++MipId)
        {
            assert(Desc->Format == DXGI_FORMAT_R8G8B8A8_UNORM);
            auto* PrevFootPrint = &MipFootPrints[MipId - 1];
            auto* CurrFootPrint = &MipFootPrints[MipId];

            texel_rgba8* SrcTexelBase = MipId == 1 ? static_cast<texel_rgba8*>(Texels) : SrcMipStart;
            for (u32 Y = 0; Y < CurrFootPrint->Footprint.Height; ++Y)
            {
                for (u32 X = 0; X < CurrFootPrint->Footprint.Width; ++X)
                {
                    texel_rgba8* DstTexel = DstMipStart + Y * CurrFootPrint->Footprint.Width + X;

                    texel_rgba8* SrcTexel00 = SrcTexelBase + (2 * Y + 0) * PrevFootPrint->Footprint.Width + 2 * X + 0;
                    texel_rgba8* SrcTexel01 = SrcTexelBase + (2 * Y + 0) * PrevFootPrint->Footprint.Width + 2 * X + 1;
                    texel_rgba8* SrcTexel10 = SrcTexelBase + (2 * Y + 1) * PrevFootPrint->Footprint.Width + 2 * X + 0;
                    texel_rgba8* SrcTexel11 = SrcTexelBase + (2 * Y + 1) * PrevFootPrint->Footprint.Width + 2 * X + 1;

                    DstTexel->Red = static_cast<u8>(round(static_cast<float>(SrcTexel00->Red + SrcTexel01->Red + SrcTexel10->Red + SrcTexel11->Red) / 4.0f));
                    DstTexel->Green = static_cast<u8>(round(static_cast<float>(SrcTexel00->Green + SrcTexel01->Green + SrcTexel10->Green + SrcTexel11->Green) / 4.0f));
                    DstTexel->Blue = static_cast<u8>(round(static_cast<float>(SrcTexel00->Blue + SrcTexel01->Blue + SrcTexel10->Blue + SrcTexel11->Blue) / 4.0f));
                    DstTexel->Alpha = static_cast<u8>(round(static_cast<float>(SrcTexel00->Alpha + SrcTexel01->Alpha + SrcTexel10->Alpha + SrcTexel11->Alpha) / 4.0f));
                }
            }

            {
                texel_rgba8* SrcRowY = DstMipStart;
                u8* DstRowY = UploadTexels + CurrFootPrint->Offset;
                CurrFootPrint->Offset += UploadOffset;

                for (u32 Y = 0; Y < CurrFootPrint->Footprint.Height; ++Y)
                {
                    memcpy(DstRowY, SrcRowY, BytesPerPixel * CurrFootPrint->Footprint.Width);
                    DstRowY += CurrFootPrint->Footprint.RowPitch;
                    SrcRowY += CurrFootPrint->Footprint.Width;
                }
            }

            SrcMipStart += PrevFootPrint->Footprint.Width * PrevFootPrint->Footprint.Height;
            DstMipStart += CurrFootPrint->Footprint.Width * CurrFootPrint->Footprint.Height;
        }
    }

    Result = Dx12CreateResource(&TextureArena, Desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);

    for (u32 MipId = 0; MipId < Desc->MipLevels; ++MipId)
    {
        D3D12_TEXTURE_COPY_LOCATION SrcRegion = {};
        SrcRegion.pResource = UploadArena.GpuBuffer;
        SrcRegion.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        SrcRegion.PlacedFootprint = MipFootPrints[MipId];

        D3D12_TEXTURE_COPY_LOCATION DstRegion = {};
        DstRegion.pResource = Result;
        DstRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        DstRegion.SubresourceIndex = MipId;

        CommandList->CopyTextureRegion(&DstRegion, 0, 0, 0, &SrcRegion, nullptr);
    }

    set_barrier(D3D12_RESOURCE_STATE_COPY_DEST, InitialState, Result);

    return Result;
}

void Renderer::Dx12CreateTexture(u32 Width, u32 Height, u8* Texels, ID3D12Resource** OutResource, D3D12_GPU_DESCRIPTOR_HANDLE* OutDescriptor)
{
    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    Desc.Width = Width;
    Desc.Height = Height;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = u32(ceil(log2(max(Desc.Width, Desc.Height))) + 1);
    Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    *OutResource = Dx12CreateTextureAsset(&Desc,
                                          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                          Texels);

    D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
    SrvDesc.Format = Desc.Format;
    SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SrvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 1, 2, 3);
    SrvDesc.Texture2D.MostDetailedMip = 0;
    SrvDesc.Texture2D.MipLevels = Desc.MipLevels;
    SrvDesc.Texture2D.PlaneSlice = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = {};
    ShaderDescHeap.allocate(&CpuDescriptor, OutDescriptor);
    Device->CreateShaderResourceView(*OutResource, &SrvDesc, CpuDescriptor);
}

void Renderer::upload_transform_buffer(ID3D12Resource* Resource, m4 WTransform, m4 VPTransform, f32 Shininess, f32 SpecularStrength)
{
    transform_buffer_cpu TransformBufferCopy = {};
    TransformBufferCopy.WTransform = WTransform;
    TransformBufferCopy.WVPTransform = VPTransform * WTransform;
    TransformBufferCopy.NormalWTransform = Transpose(Inverse(WTransform));
    TransformBufferCopy.Shininess = Shininess;
    TransformBufferCopy.SpecularStrength = SpecularStrength;
    Dx12CopyDataToBuffer(
                         D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                         D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                         &TransformBufferCopy,
                         sizeof(TransformBufferCopy),
                         Resource);
}

void Renderer::render_model(asset* Model)
{
    CommandList->SetGraphicsRootDescriptorTable(2, Model->TransformDescriptor);

    D3D12_INDEX_BUFFER_VIEW View = {};
    View.BufferLocation = Model->Model.GpuIndexBuffer->GetGPUVirtualAddress();
    View.SizeInBytes = sizeof(u32) * Model->Model.IndexCount;
    View.Format = DXGI_FORMAT_R32_UINT;
    CommandList->IASetIndexBuffer(&View);

    D3D12_VERTEX_BUFFER_VIEW Views[1] = {};
    Views[0].BufferLocation = Model->Model.GpuVertexBuffer->GetGPUVirtualAddress();
    Views[0].SizeInBytes = sizeof(vertex) * Model->Model.VertexCount;
    Views[0].StrideInBytes = sizeof(vertex);
    CommandList->IASetVertexBuffers(0, 1, Views);

    for (u32 MeshId = 0; MeshId < Model->Model.NumMeshes; ++MeshId)
    {
        mesh* CurrMesh = Model->Model.MeshArray + MeshId;
        texture* CurrTexture = Model->Model.TextureArray + CurrMesh->TextureId;
                        
        CommandList->SetGraphicsRootDescriptorTable(0, CurrTexture->GpuDescriptor);
        CommandList->DrawIndexedInstanced(CurrMesh->IndexCount, 1, CurrMesh->IndexOffset, CurrMesh->VertexOffset, 0);
    }
}

void Renderer::draw_objects()
{
    for (size_t i = 0; i < object.size(); i++)
    {
        render_model(&object[i]);
    }
}

D3D12_SHADER_BYTECODE Renderer::Dx12LoadShader(const char* FileName)
{
    D3D12_SHADER_BYTECODE Result = {};

    FILE* File;
    errno_t err = fopen_s(&File, FileName, "rb");
    Assert(err == 0 && File != NULL);

    fseek(File, 0, SEEK_END);
    Result.BytecodeLength = ftell(File);
    fseek(File, 0, SEEK_SET);

    void* FileData = malloc(Result.BytecodeLength);
    fread(FileData, Result.BytecodeLength, 1, File);
    Result.pShaderBytecode = FileData;

    fclose(File);

    return Result;
}

Renderer::Renderer(HWND WindowHandle, u32 Width, u32 Height)
{

    RenderWidth = Width;
    RenderHeight = Height;

    IDXGIFactory2* Factory = 0;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&Factory)));
 
    ID3D12Debug1* Debug;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
    Debug->EnableDebugLayer();
    Debug->SetEnableGPUBasedValidation(true);
    
    for (u32 AdapterIndex = 0;
         Factory->EnumAdapters1(AdapterIndex, &Adapter) != DXGI_ERROR_NOT_FOUND;
         ++AdapterIndex)
    {
        DXGI_ADAPTER_DESC1 Desc;
        Adapter->GetDesc1(&Desc);

        if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device))))
        {
            break;
        }
    }
    
    {
        D3D12_COMMAND_QUEUE_DESC Desc = {};
        Desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        ThrowIfFailed(Device->CreateCommandQueue(&Desc, IID_PPV_ARGS(&CommandQueue)));
    }

    {
        DXGI_SWAP_CHAIN_DESC1 Desc = {};
        Desc.Width = Width;
        Desc.Height = Height;
        Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.SampleDesc.Count = 1;
        Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        Desc.BufferCount = 2;
        Desc.Scaling = DXGI_SCALING_STRETCH;
        Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

        ThrowIfFailed(Factory->CreateSwapChainForHwnd(CommandQueue, WindowHandle, &Desc, nullptr, nullptr, &SwapChain));

        CurrentFrameIndex = 0;
        ThrowIfFailed(SwapChain->GetBuffer(0, IID_PPV_ARGS(&FrameBuffers[0])));
        ThrowIfFailed(SwapChain->GetBuffer(1, IID_PPV_ARGS(&FrameBuffers[1])));
    }

    ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)));
    ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, 0, IID_PPV_ARGS(&CommandList)));
    ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
    FenceValue = 0;

    RtvArena = Dx12ArenaCreate(D3D12_HEAP_TYPE_DEFAULT,
                                      MegaBytes(50),
                                      D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);

    UploadArena = Dx12UploadArenaCreate(MegaBytes(300));
    
    BufferArena = Dx12ArenaCreate(D3D12_HEAP_TYPE_DEFAULT,
                                         MegaBytes(100),
                                         D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

    TextureArena = Dx12ArenaCreate(D3D12_HEAP_TYPE_DEFAULT,
                                          MegaBytes(300),
                                          D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);

    RtvHeap.init_heap(Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);
    DsvHeap.init_heap(Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    ShaderDescHeap.init_heap(Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 50, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    
    {
        D3D12_RESOURCE_DESC Desc = {};
        Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        Desc.Width = Width;
        Desc.Height = Height;
        Desc.DepthOrArraySize = 1;
        Desc.MipLevels = 1;
        Desc.Format = DXGI_FORMAT_D32_FLOAT;
        Desc.SampleDesc.Count = 1;
        Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = Desc.Format;
        ClearValue.DepthStencil.Depth = 1;
        
        DepthBuffer = Dx12CreateResource(&RtvArena, &Desc,
                                                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                &ClearValue);
        DsvHeap.allocate(&DepthDescriptor, 0);
        Device->CreateDepthStencilView(DepthBuffer, 0, DepthDescriptor);
    }
    
    {
        RtvHeap.allocate(FrameBufferDescriptors + 0, 0);
        Device->CreateRenderTargetView(FrameBuffers[0], nullptr, FrameBufferDescriptors[0]);
        RtvHeap.allocate(FrameBufferDescriptors + 1, 0);
        Device->CreateRenderTargetView(FrameBuffers[1], nullptr, FrameBufferDescriptors[1]);
    }
 
    {
        // NOTE: Творимо графічний підпис
        {
            D3D12_ROOT_PARAMETER RootParameters[3] = {};

            // NOTE: Творимо першу таблицю дескрипторів
            D3D12_DESCRIPTOR_RANGE Table1Range[1] = {};
            {
                Table1Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                Table1Range[0].NumDescriptors = 1;
                Table1Range[0].BaseShaderRegister = 0;
                Table1Range[0].RegisterSpace = 0;
                Table1Range[0].OffsetInDescriptorsFromTableStart = 0;

                RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                RootParameters[0].DescriptorTable.NumDescriptorRanges = ArrayCount(Table1Range);
                RootParameters[0].DescriptorTable.pDescriptorRanges = Table1Range;
                RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }

            // NOTE: Таблиця для буфери світла
            D3D12_DESCRIPTOR_RANGE Table2Range[2] = {};
            {
                Table2Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                Table2Range[0].NumDescriptors = 1;
                Table2Range[0].BaseShaderRegister = 1;
                Table2Range[0].RegisterSpace = 0;
                Table2Range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                // NOTE: Для буфер point lights
                Table2Range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                Table2Range[1].NumDescriptors = 1;
                Table2Range[1].BaseShaderRegister = 1;
                Table2Range[1].RegisterSpace = 0;
                Table2Range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                RootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                RootParameters[1].DescriptorTable.NumDescriptorRanges = ArrayCount(Table2Range);
                RootParameters[1].DescriptorTable.pDescriptorRanges = Table2Range;
                RootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }

            // NOTE: Таблиця для буфера перетворення
            D3D12_DESCRIPTOR_RANGE Table3Range[1] = {};
            {
                Table3Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                Table3Range[0].NumDescriptors = 1;
                Table3Range[0].BaseShaderRegister = 0;
                Table3Range[0].RegisterSpace = 0;
                Table3Range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                RootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                RootParameters[2].DescriptorTable.NumDescriptorRanges = ArrayCount(Table3Range);
                RootParameters[2].DescriptorTable.pDescriptorRanges = Table3Range;
                RootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }
            
            D3D12_STATIC_SAMPLER_DESC StaticSamplerDesc = {};
            StaticSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
            StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            StaticSamplerDesc.MaxAnisotropy = 16.0f;
            StaticSamplerDesc.MinLOD = 0;
            StaticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
            StaticSamplerDesc.ShaderRegister = 0;
            StaticSamplerDesc.RegisterSpace = 0;
            StaticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            
            D3D12_ROOT_SIGNATURE_DESC SignatureDesc = {};
            SignatureDesc.NumParameters = ArrayCount(RootParameters);
            SignatureDesc.pParameters = RootParameters;
            SignatureDesc.NumStaticSamplers = 1;
            SignatureDesc.pStaticSamplers = &StaticSamplerDesc;
            SignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            
            ID3DBlob* SerializedRootSig = 0;
            ID3DBlob* ErrorBlob = 0;
            ThrowIfFailed(D3D12SerializeRootSignature(&SignatureDesc,
                                                      D3D_ROOT_SIGNATURE_VERSION_1_0,
                                                      &SerializedRootSig,
                                                      &ErrorBlob));
            ThrowIfFailed(Device->CreateRootSignature(0,
                                                      SerializedRootSig->GetBufferPointer(),
                                                      SerializedRootSig->GetBufferSize(),
                                                      IID_PPV_ARGS(&ModelRootSignature)));

            if (SerializedRootSig)
            {
                SerializedRootSig->Release();
            }
            if (ErrorBlob)
            {
                ErrorBlob->Release();
            }
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
        Desc.pRootSignature = ModelRootSignature;
        
        Desc.VS = Dx12LoadShader("ModelVsMain.shader");
        Desc.PS = Dx12LoadShader("ModelPsMain.shader");

        Desc.BlendState.RenderTarget[0].BlendEnable = true;
        Desc.BlendState.RenderTarget[0].LogicOpEnable = false;
        Desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        Desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        Desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        Desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        Desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        Desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        Desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        Desc.SampleMask = 0xFFFFFFFF;

        Desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        Desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        Desc.RasterizerState.FrontCounterClockwise = FALSE;
        Desc.RasterizerState.DepthClipEnable = TRUE;

        Desc.DepthStencilState.DepthEnable = TRUE;
        Desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        
        D3D12_INPUT_ELEMENT_DESC InputElementDescs[3] = {};
        InputElementDescs[0].SemanticName = "POSITION";
        InputElementDescs[0].SemanticIndex = 0;
        InputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        InputElementDescs[0].InputSlot = 0;
        InputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

        InputElementDescs[1].SemanticName = "TEXCOORD";
        InputElementDescs[1].SemanticIndex = 0;
        InputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
        InputElementDescs[1].InputSlot = 0;
        InputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

        InputElementDescs[2].SemanticName = "NORMAL";
        InputElementDescs[2].SemanticIndex = 0;
        InputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        InputElementDescs[2].InputSlot = 0;
        InputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElementDescs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        
        Desc.InputLayout.pInputElementDescs = InputElementDescs;
        Desc.InputLayout.NumElements = ArrayCount(InputElementDescs);

        Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        Desc.NumRenderTargets = 1;
        Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        Desc.SampleDesc.Count = 1;
        
        ThrowIfFailed(Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&ModelPipeline)));
    }


    Dx12CreateConstantBuffer(sizeof(transform_buffer_cpu),
                             &sponza.TransformBuffer,
                             &sponza.TransformDescriptor);

    Dx12CreateConstantBuffer(sizeof(transform_buffer_cpu),
        &sphere.TransformBuffer,
        &sphere.TransformDescriptor);

    Dx12CreateConstantBuffer(sizeof(transform_buffer_cpu),
        &cube.TransformBuffer,
        &cube.TransformDescriptor);

    Dx12CreateConstantBuffer(sizeof(dir_light_buffer_cpu),
                             &DirLightBuffer, &LightDescriptor);

    {
        D3D12_RESOURCE_DESC Desc = {};
        Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        Desc.Width = sizeof(point_light_cpu) * 100;
        Desc.Height = 1;
        Desc.DepthOrArraySize = 1;
        Desc.MipLevels = 1;
        Desc.Format = DXGI_FORMAT_UNKNOWN;
        Desc.SampleDesc.Count = 1;
        Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        PointLightBuffer = Dx12CreateResource(&BufferArena,
                                              &Desc,
                                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                              0);
        
        D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
        SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
        SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SrvDesc.Buffer.FirstElement = 0;
        SrvDesc.Buffer.NumElements = 100;
        SrvDesc.Buffer.StructureByteStride = sizeof(point_light_cpu);

        D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = {};
        ShaderDescHeap.allocate(&CpuDescriptor, 0);
        Device->CreateShaderResourceView(PointLightBuffer, &SrvDesc, CpuDescriptor);
    }

}
