struct VSInput
{
    float3 pos;
};

struct VSOutput
{
    float4 pos : POSITION0;
    float3 color;
};

struct GSOutput
{
    float4 pos : SV_Position;
    float3 color;
    float3 barycoords;
};

struct PushConstantData
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
[[vk::push_constant]] PushConstantData pcData;

static float3 bc[3] =
{
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0
};

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    float3 worldPos = mul(pcData.model, float4(input.pos, 1.0)).xyz;
    output.pos = mul(pcData.proj, mul(pcData.view, float4(worldPos, 1.0)));
    output.color = worldPos;
    return output;
}

[shader("geometry")]
[maxvertexcount(3)]
void geometryMain(triangle VSOutput input[3], inout TriangleStream<GSOutput> outStream)
{
    for (int i = 0; i < 3; i++)
    {
        GSOutput output;
        output.pos = input[i].pos;
        output.color = input[i].color;
        output.barycoords = bc[i];
        outStream.Append(output);
    }
    outStream.RestartStrip();
}

[shader("fragment")]
float4 fragmentMain(GSOutput input)
{
    float3 a3 = smoothstep(float3(0.0), fwidth(input.barycoords) * 1.0, input.barycoords);
    float edgeFactor = min(min(a3.x, a3.y), a3.z);
    return float4(lerp(float3(0.0), input.color, edgeFactor), 1.0);
}
