static const float PI = 3.14159265359;

#include "pbr.hlsl"

ps_input ModelVsMain(vs_input Input)
{
    ps_input Result;

    Result.Position = mul(RenderUniforms.WVPTransform, float4(Input.Position, 1.0f));
    Result.WorldPos = mul(RenderUniforms.WTransform, float4(Input.Position, 1.0f)).xyz;
    Result.Uv = Input.Uv;
    Result.Normal = normalize(mul(RenderUniforms.NormalWTransform, float4(Input.Normal, 0.0f)).xyz);

    return Result;
}

struct ps_output
{
    float4 Color : SV_TARGET0;
};

float3 EvaluatePhong(float3 SurfaceColor, float3 SurfaceNormal, float3 SurfaceWorldPos,
                     float SurfaceShininess, float SurfaceSpecularStrength,
                     float3 LightDirection, float3 LightColor, float LightAmbientIntensity)
{
    float3 NegativeLightDir = -LightDirection;
    float AccumIntensity = LightAmbientIntensity;
    
    // NOTE: Дифузне відбиття
    {
        float DiffuseIntensity = max(0, dot(SurfaceNormal, NegativeLightDir));
        AccumIntensity += DiffuseIntensity;
    }

    float SpecularIntensity = 0;
    {
        float3 CameraDirection = normalize(DirLightUniforms.CameraPos - SurfaceWorldPos);
        float3 HalfVector = normalize(NegativeLightDir + CameraDirection);
        SpecularIntensity = SurfaceSpecularStrength * pow(max(0, dot(HalfVector, SurfaceNormal)), SurfaceShininess);
    }
    
    float3 MixedColor = LightColor * SurfaceColor;
    float3 Result = AccumIntensity * MixedColor + SpecularIntensity * LightColor;

    return Result;
}

ps_output ModelPsMain(ps_input Input)
{
    ps_output Result;
    point_light PointLight = PointLightBuffer[0];
    
    float4 SurfaceColor = Texture.Sample(BilinearSampler, Input.Uv);
    float3 SurfaceNormal = normalize(Input.Normal);
    if (SurfaceColor.a == 0.0f)
    {
        discard;
    }

    Result.Color.rgb = float3(0, 0, 0);

    Result.Color.rgb = EvaluatePhong(SurfaceColor.rgb, SurfaceNormal, Input.WorldPos,
                                     RenderUniforms.Shininess, RenderUniforms.SpecularStrength,
                                     DirLightUniforms.Direction, DirLightUniforms.Color, DirLightUniforms.AmbientIntensity);

    

    float3 VectorToLight = Input.WorldPos - PointLight.Pos;
    float Radius = length(VectorToLight);
    float3 AttenuatedLight = PointLight.Color / (PointLight.DivisorConstant + Radius * Radius);

    VectorToLight /= Radius;
    Result.Color.rgb += EvaluatePhong(SurfaceColor.rgb, SurfaceNormal, Input.WorldPos,
                                          RenderUniforms.Shininess, RenderUniforms.SpecularStrength,
                                          VectorToLight, AttenuatedLight, 0);
    
    Result.Color.a = SurfaceColor.a;
    
    return Result;
}