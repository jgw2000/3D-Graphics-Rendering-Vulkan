
static float3 position[8] = {
    -100.0, -100.0, -100.0,
    100.0, -100.0, -100.0,
    100.0, 100.0, -100.0,
    -100.0, 100.0, -100.0,

    -100.0, -100.0, 100.0,
    100.0, -100.0, 100.0,
    100.0, 100.0, 100.0,
    -100.0, 100.0, 100.0,
};

static uint indices[36] = {
    1, 5, 6, 6, 2, 1,   // +x
    4, 0, 3, 3, 7, 4,   // -x
    3, 2, 6, 6, 7, 3,   // +y
    4, 5, 1, 1, 0, 4,   // -y
    7, 6, 5, 5, 4, 7,   // +z
    0, 1, 2, 2, 3, 0,   // -z
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 dir;
};

struct PushConstantData
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
    float3 cameraPos;
};
[[vk::push_constant]] PushConstantData pcData;

layout(binding = 0) Sampler2D samplerColor;
layout(binding = 1) SamplerCube samplerSkybox;

[shader("vertex")]
VSOutput vertexMain(uint vertexID : SV_VertexID)
{
    VSOutput output;

    uint idx = indices[vertexID];
    float4 worldPos = float4(pcData.cameraPos + position[idx], 1.0f);
    output.pos = mul(pcData.proj, mul(pcData.view, worldPos));
    output.dir = position[idx];

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    float3 dir = normalize(input.dir);
    return samplerSkybox.Sample(dir);
}
