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

#include "graphics_math.h"
#include "graphics_math.cpp"
#include "Renderer.hpp"

struct camera
{
    b32 PrevMouseDown;
    v2 PrevMousePos;

    f32 Yaw;
    f32 Pitch;
    b32 WDown;
    b32 ADown;
    b32 SDown;
    b32 DDown;

    v3 Pos;

    m4 update_camera(b32 MouseDown, v2 CurrMousePos, f32 FrameTime)
    {

        if (MouseDown)
        {
            if (!PrevMouseDown)
            {
                PrevMousePos = CurrMousePos;
            }

            v2 MouseDelta = CurrMousePos - PrevMousePos;
            Pitch += MouseDelta.y;
            Yaw += MouseDelta.x;

            PrevMousePos = CurrMousePos;
        }

        PrevMouseDown = MouseDown;

        m4 YawTransform = RotationMatrix(0, Yaw, 0);
        m4 PitchTransform = RotationMatrix(Pitch, 0, 0);
        m4 CameraAxisTransform = YawTransform * PitchTransform;

        v3 Right = Normalize((CameraAxisTransform * V4(1, 0, 0, 0)).xyz);
        v3 Up = Normalize((CameraAxisTransform * V4(0, 1, 0, 0)).xyz);
        v3 LookAt = Normalize((CameraAxisTransform * V4(0, 0, 1, 0)).xyz);

        m4 CameraViewTransform = IdentityM4();

        CameraViewTransform.v[0].x = Right.x;
        CameraViewTransform.v[1].x = Right.y;
        CameraViewTransform.v[2].x = Right.z;

        CameraViewTransform.v[0].y = Up.x;
        CameraViewTransform.v[1].y = Up.y;
        CameraViewTransform.v[2].y = Up.z;

        CameraViewTransform.v[0].z = LookAt.x;
        CameraViewTransform.v[1].z = LookAt.y;
        CameraViewTransform.v[2].z = LookAt.z;

        if (WDown)
        {
            Pos += LookAt * FrameTime * 0.2;
        }
        if (SDown)
        {
            Pos -= LookAt * FrameTime * 0.2;
        }
        if (DDown)
        {
            Pos += Right * FrameTime * 0.2;
        }
        if (ADown)
        {
            Pos -= Right * FrameTime * 0.2;
        }

        return CameraViewTransform * TranslationMatrix(-Pos);
    }
};

struct global_state
{
    b32 IsRunning;
    HWND WindowHandle;

    camera Camera;

};

#define WIN32_GRAPHICS_H
#endif
