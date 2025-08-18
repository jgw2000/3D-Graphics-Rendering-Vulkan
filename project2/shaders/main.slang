struct VSInput
{
    float3 pos;
    float3 normal;
    float2 uv;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv;
    float3 worldPos;
    float3 worldNormal;
};

struct PushConstantData
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
    float4 cameraPos;
};
[[vk::push_constant]] PushConstantData pcData;

layout(binding = 0) Sampler2D samplerColor;
layout(binding = 1) SamplerCube samplerSkybox;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    output.worldPos = mul(pcData.model, float4(input.pos, 1.0)).xyz;
    output.worldNormal = mul(float3x3(pcData.model), input.normal);
    output.uv = input.uv;
    output.pos = mul(pcData.proj, mul(pcData.view, float4(output.worldPos, 1.0)));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    float3 n = normalize(input.worldNormal);
    float3 v = normalize(pcData.cameraPos.xyz - input.worldPos);
    float3 reflection = -normalize(reflect(v, n));

    float4 colorRefl = samplerSkybox.Sample(reflection);
    float4 ka = colorRefl * 0.3f;

    float NdotL = clamp(dot(n, normalize(float3(-1, 1, 1))), 0.1, 1.0);
    float4 kd = samplerColor.Sample(input.uv) * NdotL;

    return ka + kd;
}
