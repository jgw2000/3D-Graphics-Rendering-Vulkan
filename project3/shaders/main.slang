struct VSInput
{
    float3 pos;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 color;
};

struct PushConstantData
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
[[vk::push_constant]] PushConstantData pcData;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    float3 worldPos = mul(pcData.model, float4(input.pos, 1.0)).xyz;
    output.pos = mul(pcData.proj, mul(pcData.view, float4(worldPos, 1.0)));
    output.color = input.pos;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    return float4(input.color, 1);
}
