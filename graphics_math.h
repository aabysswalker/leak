#if !defined(GRAPHICS_MATH_H)

global f32 Pi32 = 3.14159265359f;

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>

union v2
{
    struct
    {
        f32 x, y;
    };

    f32 e[2];
};

struct v3
{
    union
    {
        struct
        {
            f32 x, y, z;
        };

        struct
        {
            f32 r, g, b;
        };

        struct
        {
            v2 xy;
            f32 Ignored0;
        };

        struct
        {
            f32 Ignored1;
            v2 yz;
        };

        f32 e[3];
    };

    f32& operator[](i64 Index)
    {
        return e[Index];
    }
};


struct v4
{
    union
    {
        struct
        {
            f32 x, y, z, w;
        };

        struct
        {
            f32 r, g, b, a;
        };

        struct
        {
            v3 xyz;
            f32 Ignored0;
        };

        struct
        {
            v2 xy;
            v2 Ignored1;
        };

        struct
        {
            f32 Ignored3;
            v3 yzw;
        };
        
        f32 e[4];
    };

    f32& operator[](i64 Index)
    {
        return e[Index];
    }
};

struct m4
{
    union
    {
        v4 v[4];
        f32 e[16];
    };
        
    v4& operator[](i64 Index)
    {
        return v[Index];
    }
};

#define GRAPHICS_MATH_H
#endif
