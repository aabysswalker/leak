
#include <cmath>
#include <windows.h>

#include "win32_graphics.h"
#include "graphics_math.cpp"

#include "Renderer.cpp"
#include "assets.cpp"


global global_state GlobalState;

LRESULT Win32WindowCallBack(
  HWND WindowHandle,
  UINT Message,
  WPARAM WParam,
  LPARAM LParam
)
{
    LRESULT Result = {};

    switch (Message)
    {
        case WM_DESTROY:
        case WM_CLOSE:
        {
            GlobalState.IsRunning = false;
        } break;

        default:
        {
            Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
        } break;
    }
    
    return Result;
}

void ProcessPendingMessages()
{
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
                GlobalState.Camera.WDown = IsDown;
            } break;

            case 'A':
            {
                GlobalState.Camera.ADown = IsDown;
            } break;

            case 'S':
            {
                GlobalState.Camera.SDown = IsDown;
            } break;

            case 'D':
            {
                GlobalState.Camera.DDown = IsDown;
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
        WindowClass.lpszClassName = "Leak Engine";
        if (!RegisterClassA(&WindowClass))
        {
            InvalidCodePath;
        }

        GlobalState.WindowHandle = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Leak Engine",
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

    Renderer Dx12Rasterizer = Renderer(GlobalState.WindowHandle, 1920, 1080);

    Dx12Rasterizer.cube.Model = AssetCreateCube(&Dx12Rasterizer);
    Dx12Rasterizer.sponza.Model = AssetLoadModel(&Dx12Rasterizer, "Sponza\\", "Sponza.gltf");
    Dx12Rasterizer.sphere.Model = AssetCreateCube(&Dx12Rasterizer);


    Dx12Rasterizer.object.emplace_back(Dx12Rasterizer.cube);
    Dx12Rasterizer.object.emplace_back(Dx12Rasterizer.sponza);
    Dx12Rasterizer.object.emplace_back(Dx12Rasterizer.sphere);

    GlobalState.Camera.Pos = V3(0, 0.06, 0.7);

    Dx12Rasterizer.execute(true);
    
    LARGE_INTEGER BeginTime = {};
    LARGE_INTEGER EndTime = {};
    Assert(QueryPerformanceCounter(&BeginTime));
    
    while (GlobalState.IsRunning)
    {
        Assert(QueryPerformanceCounter(&EndTime));
        f32 FrameTime = f32(EndTime.QuadPart - BeginTime.QuadPart) / f32(TimerFrequency.QuadPart);
        BeginTime = EndTime;
        
        ProcessPendingMessages();
        RECT ClientRect = {};
        Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        u32 ClientWidth = ClientRect.right - ClientRect.left;
        u32 ClientHeight = ClientRect.bottom - ClientRect.top;
        f32 AspectRatio = f32(ClientWidth) / f32(ClientHeight);


        m4 CameraTransform = IdentityM4();
        {
            b32 MouseDown = false;
            v2 CurrMousePos = {};
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

            CameraTransform = GlobalState.Camera.update_camera(MouseDown, CurrMousePos, FrameTime);
        }
                        
        m4 sponza_model = (TranslationMatrix(0, 0, 1) * RotationMatrix(0, Pi32 * 0.5f, 0) * ScaleMatrix(1, 1, 1));
        m4 view_projection_matrix = (PerspectiveMatrix(60.0f, AspectRatio, 0.01f, 1000.0f) * CameraTransform);

        local_global f32 Time = 0.0f;
        Time += FrameTime;
        point_light_cpu PointLights = {};
        PointLights.Pos = V3(0, 0.065f + 0.04f * sin(Time), 1);
        PointLights.DivisorConstant = 0.4f;
        PointLights.Color = 0.4f * V3(1.0f, 0.3f, 0.3f);

        Dx12Rasterizer.Dx12CopyDataToBuffer(
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &PointLights,
            sizeof(PointLights),
            Dx12Rasterizer.PointLightBuffer);

        m4 model_matrix = TranslationMatrix(PointLights.Pos) * ScaleMatrix(0.01f, 0.01f, 0.01f);

        m4 sphere_matrix = TranslationMatrix(V3(0, 0.09f, .8)) * ScaleMatrix(0.01f, 0.01f, 0.01f);

        Dx12Rasterizer.upload_transform_buffer(Dx12Rasterizer.sphere.TransformBuffer, sphere_matrix, view_projection_matrix, 0.001f, 0.0f);
        Dx12Rasterizer.upload_transform_buffer(Dx12Rasterizer.cube.TransformBuffer, model_matrix, view_projection_matrix, 0.001f, 0.0f);
        Dx12Rasterizer.upload_transform_buffer(Dx12Rasterizer.sponza.TransformBuffer, sponza_model, view_projection_matrix, 100.0f, 1.0f);
        

        {
            dir_light_buffer_cpu LightBufferCopy = {};
            LightBufferCopy.LightAmbientIntensity = 0.4f;
            LightBufferCopy.LightColor = 0.3f * V3(1.0f, 1.0f, 1.0f);
            LightBufferCopy.LightDirection = Normalize(V3(0, -1.0f, 0.0f));
            LightBufferCopy.CameraPos = GlobalState.Camera.Pos;
            Dx12Rasterizer.Dx12CopyDataToBuffer(
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                &LightBufferCopy,
                sizeof(LightBufferCopy),
                Dx12Rasterizer.DirLightBuffer);
        }
        
        Dx12Rasterizer.set_barrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, Dx12Rasterizer.FrameBuffers[Dx12Rasterizer.CurrentFrameIndex]);

        const FLOAT Color[4] = { 1, 0, 1, 1 };
        Dx12Rasterizer.clear_render_view(Color);

        Dx12Rasterizer.set_viewport();

        Dx12Rasterizer.CommandList->SetGraphicsRootSignature(Dx12Rasterizer.ModelRootSignature);
        Dx12Rasterizer.CommandList->SetPipelineState(Dx12Rasterizer.ModelPipeline);
  
        Dx12Rasterizer.CommandList->SetDescriptorHeaps(1, &Dx12Rasterizer.ShaderDescHeap.Heap);
        Dx12Rasterizer.CommandList->SetGraphicsRootDescriptorTable(1, Dx12Rasterizer.LightDescriptor);

        Dx12Rasterizer.draw_objects();
        
        Dx12Rasterizer.set_barrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, Dx12Rasterizer.FrameBuffers[Dx12Rasterizer.CurrentFrameIndex]);

        Dx12Rasterizer.execute(false);
        Dx12Rasterizer.CurrentFrameIndex = (Dx12Rasterizer.CurrentFrameIndex + 1) % 2;
    }
    
    return 0;
}
