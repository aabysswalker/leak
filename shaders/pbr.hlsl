float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265359 * denom * denom;

    return a2 / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float4 EvaluatePBR(ps_input Input, point_light PointLight, float3 Color, float3 Normal, float3 MR)
{
    PointLight.Pos.z += .2;
    float3 light_position = PointLight.Pos;
    
    float3 N = normalize(Normal);
    float3 V = normalize(DirLightUniforms.CameraPos - Input.WorldPos);

    float3 albedo = Color;
    float metallic = MR.b;
    float roughness = MR.g;
    float ao = 1.0;

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    float3 Lo = float3(0.0, 0.0, 0.0);

    float3 L = normalize(light_position - Input.WorldPos);
    float3 H = normalize(V + L);
    float distance = length(light_position - Input.WorldPos);
    float attenuation = 1.0 / (distance * distance);
    float3 radiance = float3(1.0, 1.0, 1.0) * attenuation;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);

    Lo += (kD * albedo / 3.14159265359 + specular) * radiance * NdotL;

    float3 ambient = float3(0.2, 0.2, 0.2) * albedo * ao;
    float3 color = ambient + Lo;
    
    return float4(color, 1.0);
}
