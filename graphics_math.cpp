#pragma once


u64 Align(u64 Location, u64 Alignment)
{
    u64 Result = (Location + (Alignment - 1)) & (~(Alignment - 1));
    return Result;
}

i64 Sign(i64 X)
{
    i64 Result = (X > 0) - (X < 0);
    return Result;
}
//
// NOTE: V2
//

v2 V2(f32 Arg)
{
    v2 Result = {};
    Result.x = Arg;
    Result.y = Arg;
    return Result;
}

v2 V2(f32 X, f32 Y)
{
    v2 Result = {};
    Result.x = X;
    Result.y = Y;
    return Result;
}

v2 operator+(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    return Result;
}

v2 operator-(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    return Result;
}

v2 operator*(f32 A, v2 B)
{
    v2 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    return Result;
}

v2 operator*(v2 B, f32 A)
{
    v2 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    return Result;
}

v2 operator*(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    return Result;
}

v2 operator*=(v2& A, f32 B)
{
    A = A * B;
    return A;
}

v2 operator/(v2 A, f32 B)
{
    v2 Result = {};
    Result.x = A.x / B;
    Result.y = A.y / B;
    return Result;
}

v2 operator/(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    return Result;
}

v2 operator/=(v2& A, f32 B)
{
    A = A / B;
    return A;
}


v3 V3(f32 X, f32 Y, f32 Z)
{
    v3 Result = {};
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    return Result;
}

v3 V3(f32 A)
{
    v3 Result = {};
    Result.x = A;
    Result.y = A;
    Result.z = A;
    return Result;
}

v3 operator+(v3 A, v3 B)
{
    v3 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    return Result;
}

v3 operator+=(v3& A, v3 B)
{
    A = A + B;
    return A;
}

v3 operator-(v3 A)
{
    v3 Result = {};
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    return Result;
}

v3 operator-(v3 A, v3 B)
{
    v3 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    return Result;
}

v3 operator-=(v3& A, v3 B)
{
    A = A - B;
    return A;
}

v3 operator*(f32 A, v3 B)
{
    v3 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    return Result;
}

v3 operator*(v3 B, f32 A)
{
    v3 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    return Result;
}

v3 operator*=(v3& A, f32 B)
{
    A = A * B;
    return A;
}

v3 operator/(v3 B, f32 A)
{
    v3 Result = {};
    Result.x = B.x / A;
    Result.y = B.y / A;
    Result.z = B.z / A;
    return Result;
}

v3 operator/=(v3& A, f32 B)
{
    A = A / B;
    return A;
}

v3 Normalize(v3 A)
{
    f32 Length = sqrt(A.x * A.x + A.y * A.y + A.z * A.z);
    v3 Result = A / Length;
    return Result;
}

v3 Lerp(v3 A, v3 B, f32 T)
{
    v3 Result = (1.0f - T) * A + T * B;
    return Result;
}

//
// NOTE: V4
//

v4 V4(f32 X, f32 Y, f32 Z, f32 W)
{
    v4 Result = {};
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    return Result;
}

v4 V4(v3 A, f32 W)
{
    v4 Result = {};
    Result.xyz = A;
    Result.w = W;
    return Result;
}

v4 operator+(v4 A, v4 B)
{
    v4 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    return Result;
}

v4 operator*(v4 A, f32 B)
{
    v4 Result = {};
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;
    return Result;
}

v4 operator*(f32 B, v4 A)
{
    v4 Result = {};
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;
    return Result;
}

//
// NOTE: M4
//

m4 IdentityM4()
{
    m4 Result = {};
    Result.v[0].x = 1.0f;
    Result.v[1].y = 1.0f;
    Result.v[2].z = 1.0f;
    Result.v[3].w = 1.0f;
    return Result;
}

v4 operator*(m4 A, v4 B)
{
    v4 Result = A.v[0] * B.x + A.v[1] * B.y + A.v[2] * B.z + A.v[3] * B.w;
    return Result;
}

m4 operator*(m4 A, m4 B)
{
    m4 Result = {};
    Result.v[0] = A * B.v[0];
    Result.v[1] = A * B.v[1];
    Result.v[2] = A * B.v[2];
    Result.v[3] = A * B.v[3];
    return Result;
}

m4 Transpose(m4 A)
{
    m4 Result = {};
    Result[0][0] = A[0][0];
    Result[0][1] = A[1][0];
    Result[0][2] = A[2][0];
    Result[0][3] = A[3][0];
    Result[1][0] = A[0][1];
    Result[1][1] = A[1][1];
    Result[1][2] = A[2][1];
    Result[1][3] = A[3][1];
    Result[2][0] = A[0][2];
    Result[2][1] = A[1][2];
    Result[2][2] = A[2][2];
    Result[2][3] = A[3][2];
    Result[3][0] = A[0][3];
    Result[3][1] = A[1][3];
    Result[3][2] = A[2][3];
    Result[3][3] = A[3][3];

    return Result;
}

f32 Determinant3x3(v3 ColA, v3 ColB, v3 ColC)
{
    f32 Result = (ColA[0]*ColB[1]*ColC[2] + ColB[0]*ColC[1]*ColA[2] + ColC[0]*ColA[1]*ColB[2] -
                  ColC[0]*ColB[1]*ColA[2] - ColB[0]*ColA[1]*ColC[2] - ColA[0]*ColC[1]*ColB[2]);
    
    return Result;
}

f32 Determinant(m4 A)
{
    f32 Result = (A[0][0] * Determinant3x3(A[1].yzw, A[2].yzw, A[3].yzw) -
                  A[0][1] * Determinant3x3(V3(A[1].x, A[1].z, A[1].w), V3(A[2].x, A[2].z, A[2].w), V3(A[3].x, A[3].z, A[3].w)) +
                  A[0][2] * Determinant3x3(V3(A[1].x, A[1].y, A[1].w), V3(A[2].x, A[2].y, A[2].w), V3(A[3].x, A[3].y, A[3].w)) -
                  A[0][3] * Determinant3x3(A[1].xyz, A[2].xyz, A[3].xyz));
    return Result;
}

m4 Inverse(m4 A)
{
    // NOTE: https://semath.info/src/inverse-cofactor-ex4.html
    // IMPORTANT: Цей алгоритм певно повільніший як він повинен бути
    f32 OneOverDeterminant = Determinant(A);
    Assert(OneOverDeterminant != 0.0f);
    OneOverDeterminant = 1.0f / OneOverDeterminant;
    
    m4 Result = {};
    Result[0][0] = +Determinant3x3(A[1].yzw, A[2].yzw, A[3].yzw) * OneOverDeterminant;
    Result[0][1] = -Determinant3x3(A[0].yzw, A[2].yzw, A[3].yzw) * OneOverDeterminant;
    Result[0][2] = +Determinant3x3(A[0].yzw, A[1].yzw, A[3].yzw) * OneOverDeterminant;
    Result[0][3] = -Determinant3x3(A[0].yzw, A[1].yzw, A[2].yzw) * OneOverDeterminant;

    Result[1][0] = -Determinant3x3(V3(A[1].x, A[1].z, A[1].w), V3(A[2].x, A[2].z, A[2].w), V3(A[3].x, A[3].z, A[3].w)) * OneOverDeterminant;
    Result[1][1] = +Determinant3x3(V3(A[0].x, A[0].z, A[0].w), V3(A[2].x, A[2].z, A[2].w), V3(A[3].x, A[3].z, A[3].w)) * OneOverDeterminant;
    Result[1][2] = -Determinant3x3(V3(A[0].x, A[0].z, A[0].w), V3(A[1].x, A[1].z, A[1].w), V3(A[3].x, A[3].z, A[3].w)) * OneOverDeterminant;
    Result[1][3] = +Determinant3x3(V3(A[0].x, A[0].z, A[0].w), V3(A[1].x, A[1].z, A[1].w), V3(A[2].x, A[2].z, A[2].w)) * OneOverDeterminant;

    Result[2][0] = +Determinant3x3(V3(A[1].x, A[1].y, A[1].w), V3(A[2].x, A[2].y, A[2].w), V3(A[3].x, A[3].y, A[3].w)) * OneOverDeterminant;
    Result[2][1] = -Determinant3x3(V3(A[0].x, A[0].y, A[0].w), V3(A[2].x, A[2].y, A[2].w), V3(A[3].x, A[3].y, A[3].w)) * OneOverDeterminant;
    Result[2][2] = +Determinant3x3(V3(A[0].x, A[0].y, A[0].w), V3(A[1].x, A[1].y, A[1].w), V3(A[3].x, A[3].y, A[3].w)) * OneOverDeterminant;
    Result[2][3] = -Determinant3x3(V3(A[0].x, A[0].y, A[0].w), V3(A[1].x, A[1].y, A[1].w), V3(A[2].x, A[2].y, A[2].w)) * OneOverDeterminant;

    Result[3][0] = -Determinant3x3(A[1].xyz, A[2].xyz, A[3].xyz) * OneOverDeterminant;
    Result[3][1] = +Determinant3x3(A[0].xyz, A[2].xyz, A[3].xyz) * OneOverDeterminant;
    Result[3][2] = -Determinant3x3(A[0].xyz, A[1].xyz, A[3].xyz) * OneOverDeterminant;
    Result[3][3] = +Determinant3x3(A[0].xyz, A[1].xyz, A[2].xyz) * OneOverDeterminant;

    return Result;
}

m4 ScaleMatrix(f32 X, f32 Y, f32 Z)
{
    m4 Result = IdentityM4();
    Result.v[0].x = X;
    Result.v[1].y = Y;
    Result.v[2].z = Z;
    return Result;
}

m4 RotationMatrix(f32 X, f32 Y, f32 Z)
{
    m4 Result = {};

    m4 RotateX = IdentityM4();
    RotateX.v[1].y = cos(X);
    RotateX.v[2].y = -sin(X);
    RotateX.v[1].z = sin(X);
    RotateX.v[2].z = cos(X);

    m4 RotateY = IdentityM4();
    RotateY.v[0].x = cos(Y);
    RotateY.v[2].x = -sin(Y);
    RotateY.v[0].z = sin(Y);
    RotateY.v[2].z = cos(Y);

    m4 RotateZ = IdentityM4();
    RotateZ.v[0].x = cos(Z);
    RotateZ.v[1].x = -sin(Z);
    RotateZ.v[0].y = sin(Z);
    RotateZ.v[1].y = cos(Z);

    Result = RotateZ * RotateY * RotateX;
    return Result;
}

m4 TranslationMatrix(f32 X, f32 Y, f32 Z)
{
    m4 Result = IdentityM4();
    Result.v[3].xyz = V3(X, Y, Z);
    return Result;
}

m4 TranslationMatrix(v3 Pos)
{
    m4 Result = TranslationMatrix(Pos.x, Pos.y, Pos.z);
    return Result;
}

m4 PerspectiveMatrix(f32 Fov, f32 AspectRatio, f32 NearZ, f32 FarZ)
{
    m4 Result = {};

    f32 FovRadians = (Fov / 360.0f) * 2.0f * Pi32;
    
    Result.v[0].x = 1.0f / (AspectRatio * tan(FovRadians * 0.5f));
    Result.v[1].y = 1.0f / (tan(FovRadians * 0.5f));
    Result.v[2].z = -FarZ / (NearZ - FarZ);
    Result.v[3].z = NearZ * FarZ / (NearZ - FarZ);
    Result.v[2].w = 1.0f;

    return Result;
}
