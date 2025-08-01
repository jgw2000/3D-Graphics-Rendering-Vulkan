struct VSInput
{
    float3 pos;
    float3 normal;
    float2 uv;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 normal;
    float2 uv;
};

Sampler2D samplerColor;

[shader("vertex")]
VSOutput vertexMain(VSInput input, uniform float4x4 mvp)
{
    VSOutput output;
    output.pos = mul(mvp, float4(input.pos, 1.0));
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    return samplerColor.Sample(input.uv);
}
