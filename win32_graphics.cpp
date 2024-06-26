#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <cmath>
#include <windows.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_3.h>

namespace dxm = DirectX;

#if !defined(WIN32_GRAPHICS_H)

#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <stdio.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t mm;
typedef uintptr_t umm;

typedef int32_t b32;

#define global static
#define local_global static

#define snprintf _snprintf_s
#define Assert(Expression) if (!(Expression)) {__debugbreak();}
#define InvalidCodePath Assert(!"Invalid Code Path")
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define KiloBytes(Val) ((Val)*1024LL)
#define MegaBytes(Val) (KiloBytes(Val)*1024LL)
#define GigaBytes(Val) (MegaBytes(Val)*1024LL)
#define TeraBytes(Val) (GigaBytes(Val)*1024LL)

int Align(int Location, int Alignment)
{
    int Result = (Location + (Alignment - 1)) & (~(Alignment - 1));
    return Result;
}

int Sign(int X)
{
    int Result = (X > 0) - (X < 0);
    return Result;
}

#include "assets.h"
#include "dx12_rasterizer.h"


struct camera
{
    b32 PrevMouseDown;
    dxm::XMFLOAT2 PrevMousePos;

    f32 Yaw;
    f32 Pitch;

    dxm::XMFLOAT3 Pos;
};

struct global_state
{
    b32 IsRunning;
    HWND WindowHandle;

    f32 CurrTime;

    b32 WDown;
    b32 ADown;
    b32 SDown;
    b32 DDown;
    camera Camera;

    dx12_rasterizer Dx12Rasterizer;
};

#define WIN32_GRAPHICS_H
#endif


#include "dx12_rasterizer.cpp"
#include "assets.cpp"

global global_state GlobalState;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Win32WindowCallBack(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = {};

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_DESTROY:
    case WM_CLOSE:
    {
        GlobalState.IsRunning = false;
    } break;

    default:
    {
        Result = DefWindowProcA(hWnd, msg, wParam, lParam);
    } break;
    }

    return Result;
}

int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd)
{
    GlobalState.IsRunning = true;
    LARGE_INTEGER TimerFrequency = {};
    Assert(QueryPerformanceFrequency(&TimerFrequency));

    {
        WNDCLASSA WindowClass = {};
        WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        WindowClass.lpfnWndProc = Win32WindowCallBack;
        WindowClass.hInstance = hInstance;
        WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
        WindowClass.lpszClassName = "Graphics Tutorial";
        if (!RegisterClassA(&WindowClass))
        {
            InvalidCodePath;
        }

        GlobalState.WindowHandle = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Graphics Tutorial",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            1280,
            720,
            NULL,
            NULL,
            hInstance,
            NULL);

        if (!GlobalState.WindowHandle)
        {
            InvalidCodePath;
        }
    }

    GlobalState.Dx12Rasterizer = Dx12RasterizerCreate(GlobalState.WindowHandle, 1920, 1080);

    GlobalState.Camera.Pos = dxm::XMFLOAT3(0, 0.06, 0.7);

    dx12_rasterizer* Rasterizer = &GlobalState.Dx12Rasterizer;
    dxm::XMMATRIX model = 
        dxm::XMMatrixRotationRollPitchYaw(1.2, 0, 0) *
        dxm::XMMatrixScaling(0.05, 0.05, 0.05) 
        * dxm::XMMatrixTranslation(0, .1, .8);
    dxm::XMMATRIX WTransform = 
        dxm::XMMatrixRotationRollPitchYawFromVector(dxm::XMVectorSet(0, 3.1451 * 0.5f, 0, 0)) 
        * dxm::XMMatrixScaling(1, 1, 1) 
        * dxm::XMMatrixTranslation(0, 0, 1);

    Rasterizer->objects[0].model = AssetLoadModel(Rasterizer, "data/sponza/", "Sponza.gltf", WTransform);
    Rasterizer->objects[Rasterizer->objects.size() - 1].model = AssetLoadModel(Rasterizer, "data/DamagedHelmet/", "DamagedHelmet.gltf", model, true);
    
    for (int i = 0; i < Rasterizer->objects.size(); i++)
    {
        Dx12CreateConstantBuffer(Rasterizer, sizeof(transform_buffer_cpu),
            &Rasterizer->objects[i].TransformBuffer,
            &Rasterizer->objects[i].TransformDescriptor);
    }

    ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

    ThrowIfFailed(CommandList->Close());
    Rasterizer->CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&CommandList);

    Rasterizer->FenceValue += 1;
    ThrowIfFailed(Rasterizer->CommandQueue->Signal(Rasterizer->Fence, Rasterizer->FenceValue));
    if (Rasterizer->Fence->GetCompletedValue() != Rasterizer->FenceValue)
    {

        HANDLE FenceEvent = {};
        ThrowIfFailed(Rasterizer->Fence->SetEventOnCompletion(Rasterizer->FenceValue, FenceEvent));
        WaitForSingleObject(FenceEvent, INFINITE);
    }

    ThrowIfFailed(Rasterizer->CommandAllocator->Reset());
    ThrowIfFailed(CommandList->Reset(Rasterizer->CommandAllocator, 0));

    Dx12ClearUploadArena(&Rasterizer->UploadArena);

    LARGE_INTEGER BeginTime = {};
    LARGE_INTEGER EndTime = {};
    Assert(QueryPerformanceCounter(&BeginTime));
    float sens = 0.3;
    while (GlobalState.IsRunning)
    {
        Assert(QueryPerformanceCounter(&EndTime));
        f32 FrameTime = f32(EndTime.QuadPart - BeginTime.QuadPart) / f32(TimerFrequency.QuadPart);
        BeginTime = EndTime;

        {
            char Text[256];
            snprintf(Text, sizeof(Text), "FrameTime: %f\n", FrameTime);
            OutputDebugStringA(Text);
        }

        MSG Message = {};
        while (PeekMessageA(&Message, GlobalState.WindowHandle, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
            case WM_QUIT:
            {
                GlobalState.IsRunning = false;
            } break;

            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                u32 VkCode = Message.wParam;
                b32 IsDown = !((Message.lParam >> 31) & 0x1);

                switch (VkCode)
                {
                case 'W':
                {
                    GlobalState.WDown = IsDown;
                } break;

                case 'A':
                {
                    GlobalState.ADown = IsDown;
                } break;

                case 'S':
                {
                    GlobalState.SDown = IsDown;
                } break;

                case 'D':
                {
                    GlobalState.DDown = IsDown;
                } break;
                }
            } break;

            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
            }
        }

        RECT ClientRect = {};
        Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        u32 ClientWidth = ClientRect.right - ClientRect.left;
        u32 ClientHeight = ClientRect.bottom - ClientRect.top;
        f32 AspectRatio = f32(ClientWidth) / f32(ClientHeight);

        // NOTE: Обчислюємо положення камери
        dxm::XMMATRIX CameraTransform = dxm::XMMatrixIdentity();
        {
            camera* Camera = &GlobalState.Camera;

            b32 MouseDown = false;
            dxm::XMFLOAT2 CurrMousePos = {};
            if (GetActiveWindow() == GlobalState.WindowHandle)
            {
                POINT Win32MousePos = {};
                Assert(GetCursorPos(&Win32MousePos));
                Assert(ScreenToClient(GlobalState.WindowHandle, &Win32MousePos));

                Win32MousePos.y = ClientRect.bottom - Win32MousePos.y;

                CurrMousePos.x = f32(Win32MousePos.x) / f32(ClientWidth);
                CurrMousePos.y = f32(Win32MousePos.y) / f32(ClientHeight);

                MouseDown = (GetKeyState(VK_LBUTTON) & 0x80) != 0;
            }

            if (MouseDown)
            {
                if (!Camera->PrevMouseDown)
                {
                    Camera->PrevMousePos = CurrMousePos;
                }

                dxm::XMFLOAT2 MouseDelta = dxm::XMFLOAT2(CurrMousePos.x - Camera->PrevMousePos.x, CurrMousePos.y - Camera->PrevMousePos.y);
                Camera->Pitch += MouseDelta.y;
                Camera->Yaw -= MouseDelta.x;

                Camera->PrevMousePos = CurrMousePos;
            }

            Camera->PrevMouseDown = MouseDown;

            dxm::XMMATRIX YawTransform = dxm::XMMatrixRotationRollPitchYaw(0, Camera->Yaw, 0);
            dxm::XMMATRIX PitchTransform = dxm::XMMatrixRotationRollPitchYaw(Camera->Pitch, 0, 0);
            dxm::XMMATRIX CameraAxisTransform =  PitchTransform * YawTransform;

            dxm::XMVECTOR Right = dxm::XMVector3Normalize(XMVector3TransformNormal(dxm::XMVectorSet(1, 0, 0, 0), CameraAxisTransform));
            dxm::XMVECTOR Up = dxm::XMVector3Normalize(XMVector3TransformNormal(dxm::XMVectorSet(0, 1, 0, 0), CameraAxisTransform));
            dxm::XMVECTOR LookAt = dxm::XMVector3Normalize(XMVector3TransformNormal(dxm::XMVectorSet(0, 0, 1, 0), CameraAxisTransform));

            dxm::XMMATRIX CameraViewTransform = dxm::XMMatrixIdentity();

            CameraViewTransform = dxm::XMMatrixSet(
                dxm::XMVectorGetX(Right), dxm::XMVectorGetX(Up), dxm::XMVectorGetX(LookAt), 0.0f,
                dxm::XMVectorGetY(Right), dxm::XMVectorGetY(Up), dxm::XMVectorGetY(LookAt), 0.0f,
                dxm::XMVectorGetZ(Right), dxm::XMVectorGetZ(Up), dxm::XMVectorGetZ(LookAt), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );

            

            if (GlobalState.WDown)
            {
                dxm::XMVECTOR LookAtScaled = dxm::XMVectorScale(LookAt, FrameTime * sens);
                dxm::XMStoreFloat3(&Camera->Pos, dxm::XMVectorAdd(dxm::XMLoadFloat3(&Camera->Pos), LookAtScaled));
            }
            if (GlobalState.SDown)
            {
                dxm::XMVECTOR LookAtScaled = dxm::XMVectorScale(LookAt, FrameTime * sens);
                dxm::XMStoreFloat3(&Camera->Pos, dxm::XMVectorSubtract(dxm::XMLoadFloat3(&Camera->Pos), LookAtScaled));
            }
            if (GlobalState.DDown)
            {
                dxm::XMVECTOR RightScaled = dxm::XMVectorScale(Right, FrameTime * sens); 
                dxm::XMStoreFloat3(&Camera->Pos, dxm::XMVectorAdd(dxm::XMLoadFloat3(&Camera->Pos), RightScaled));
            }
            if (GlobalState.ADown)
            {
                dxm::XMVECTOR RightScaled = dxm::XMVectorScale(Right, FrameTime * sens);
                dxm::XMStoreFloat3(&Camera->Pos, dxm::XMVectorSubtract(dxm::XMLoadFloat3(&Camera->Pos), RightScaled));
            }


            CameraTransform = dxm::XMMatrixTranslation(-Camera->Pos.x, -Camera->Pos.y, -Camera->Pos.z) * CameraViewTransform;
        }


        GlobalState.CurrTime = GlobalState.CurrTime + FrameTime;
        if (GlobalState.CurrTime > 2.0f * 3.14159f)
        {
            GlobalState.CurrTime -= 2.0f * 3.14159f;
        }


        f32 Offset = abs(sin(GlobalState.CurrTime));
        
        dxm::XMMATRIX VPTransform = CameraTransform * dxm::XMMatrixPerspectiveFovLH(dxm::XMConvertToRadians(60.0f), AspectRatio, 0.01f, 1000.0f);

        dx12_rasterizer* Rasterizer = &GlobalState.Dx12Rasterizer;
        ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

        local_global f32 Time = 0.0f;
        Time += FrameTime;
        if (Time > 2.0f * 3.1451)
        {
            Time -= 2.0f * 3.1451;
        }

        point_light_cpu PointLights[NUM_POINT_LIGHTS] = {};
        {
            PointLights[0].Pos = dxm::XMFLOAT3(0, 0.065f + 0.04f * sin(Time), 1);
            PointLights[0].DivisorConstant = 0.4f;
            PointLights[0].Color = dxm::XMFLOAT3(0.4f * 1.0f, 0.4f * 0.3f, 0.4f * 0.3f);

            PointLights[1].Pos = dxm::XMFLOAT3(0.1f * cos(Time), 0.1f, 1.1f + 0.1f * sin(Time));
            PointLights[1].DivisorConstant = 0.4f;
            PointLights[1].Color = dxm::XMFLOAT3(0.3f * 0.0f, 0.3f * 1.0f, 0.3f * 0.3f);

            Dx12CopyDataToBuffer(Rasterizer,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                PointLights,
                sizeof(PointLights),
                Rasterizer->PointLightBuffer);
        }

        {
            dir_light_buffer_cpu LightBufferCopy = {};
            LightBufferCopy.LightAmbientIntensity = 0.4f;
            LightBufferCopy.LightColor = dxm::XMFLOAT3(0.3f, 0.3f, 0.3f);
            dxm::XMStoreFloat3(&LightBufferCopy.LightDirection, dxm::XMVector3Normalize(dxm::XMVectorSet(cos(Time), -1.0f, 0.0f, 0.0f)));
            LightBufferCopy.NumPointLights = 2;
            LightBufferCopy.CameraPos = GlobalState.Camera.Pos;
            Dx12CopyDataToBuffer(Rasterizer,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                &LightBufferCopy,
                sizeof(LightBufferCopy),
                Rasterizer->DirLightBuffer);
        }

        {
            D3D12_RESOURCE_BARRIER Barrier = {};
            Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            Barrier.Transition.pResource = Rasterizer->FrameBuffers[Rasterizer->CurrentFrameIndex];
            Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            CommandList->ResourceBarrier(1, &Barrier);
        }

        const FLOAT Color[4] = { 1, 0, 1, 1 };
        CommandList->ClearRenderTargetView(Rasterizer->FrameBufferDescriptors[Rasterizer->CurrentFrameIndex], Color, 0, nullptr);
        CommandList->ClearDepthStencilView(Rasterizer->DepthDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, 0);

        CommandList->OMSetRenderTargets(1, Rasterizer->FrameBufferDescriptors + Rasterizer->CurrentFrameIndex, 0,
            &Rasterizer->DepthDescriptor);

        D3D12_VIEWPORT Viewport = {};
        Viewport.TopLeftX = 0;
        Viewport.TopLeftY = 0;
        Viewport.Width = Rasterizer->RenderWidth;
        Viewport.Height = Rasterizer->RenderHeight;
        Viewport.MinDepth = 0;
        Viewport.MaxDepth = 1;
        CommandList->RSSetViewports(1, &Viewport);

        D3D12_RECT ScissorRect = {};
        ScissorRect.left = 0;
        ScissorRect.right = Rasterizer->RenderWidth;
        ScissorRect.top = 0;
        ScissorRect.bottom = Rasterizer->RenderHeight;
        CommandList->RSSetScissorRects(1, &ScissorRect);

        CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        CommandList->SetGraphicsRootSignature(Rasterizer->ModelRootSignature);
        CommandList->SetPipelineState(Rasterizer->ModelPipeline);

        CommandList->SetDescriptorHeaps(1, &Rasterizer->ShaderDescHeap.Heap);
        CommandList->SetGraphicsRootDescriptorTable(1, Rasterizer->LightDescriptor);

        for (int i = 0; i < Rasterizer->objects.size(); i++)
        {
            Dx12UploadTransformBuffer(Rasterizer,
                Rasterizer->objects[i].TransformBuffer,
                Rasterizer->objects[i].model.transform,
                VPTransform,
                100.0f, 1.0f,
                Rasterizer->objects[i].model.is_pbr);

            Dx12RenderModel(CommandList, &Rasterizer->objects[i].model, Rasterizer->objects[i].TransformDescriptor);
        }

        {
            D3D12_RESOURCE_BARRIER Barrier = {};
            Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            Barrier.Transition.pResource = Rasterizer->FrameBuffers[Rasterizer->CurrentFrameIndex];
            Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            CommandList->ResourceBarrier(1, &Barrier);
        }

        ThrowIfFailed(CommandList->Close());
        Rasterizer->CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&CommandList);
        Rasterizer->SwapChain->Present(1, 0);

        Rasterizer->FenceValue += 1;
        ThrowIfFailed(Rasterizer->CommandQueue->Signal(Rasterizer->Fence, Rasterizer->FenceValue));
        if (Rasterizer->Fence->GetCompletedValue() != Rasterizer->FenceValue)
        {
            HANDLE FenceEvent = {};
            ThrowIfFailed(Rasterizer->Fence->SetEventOnCompletion(Rasterizer->FenceValue, FenceEvent));
            WaitForSingleObject(FenceEvent, INFINITE);
        }

        ThrowIfFailed(Rasterizer->CommandAllocator->Reset());
        ThrowIfFailed(CommandList->Reset(Rasterizer->CommandAllocator, 0));

        Rasterizer->CurrentFrameIndex = (Rasterizer->CurrentFrameIndex + 1) % 2;
        Dx12ClearUploadArena(&Rasterizer->UploadArena);
    }

    return 0;
}
