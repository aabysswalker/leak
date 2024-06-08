struct vs_input
{
    float3 Position : POSITION;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL;
};

struct ps_input
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL;
};

struct render_uniforms
{
    float4x4 WVPTransform;
    float4x4 WTransform;
    float4x4 NormalWTransform;
    float Shininess;
    float SpecularStrength;
};

struct test
{
    float field;
};

cbuffer RenderUniformsBuffer : register(b0)
{
    render_uniforms RenderUniforms;
};

cbuffer test : register(b0)
{
    test Some;
};

struct dir_light_uniforms
{
    float3 Color;
    float AmbientIntensity;
    float3 Direction;
    float3 CameraPos;

};

struct point_light
{
    float3 Pos;
    float DivisorConstant;
    float3 Color;
};

cbuffer DirLightUniformsBuffer : register(b1)
{
    dir_light_uniforms DirLightUniforms;
};

StructuredBuffer<point_light> PointLightBuffer : register(t1);

Texture2D Texture : register(t0);
SamplerState BilinearSampler : register(s0);
