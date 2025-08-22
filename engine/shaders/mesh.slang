struct VSInput
{
    float3 pos;
    float2 uv;
    float3 normal;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv;
    float3 normal;
};

struct GSOutput
{
    float4 pos : SV_Position;
    float2 uv;
    float3 normal;
    float3 barycoords;
};

struct PushConstantData
{
    float4x4 mvp;
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
    output.pos = mul(pcData.mvp, float4(input.pos, 1.0));
    output.uv = input.uv;
    output.normal = input.normal;
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
        output.uv = input[i].uv;
        output.normal = input[i].normal;
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

    float NdotL = clamp(dot(normalize(input.normal), normalize(float3(-1, -1, -1))), 0.5, 1.0);
    float3 color = float3(1.0, 1.0, 1.0) * NdotL;

    return float4(lerp(float3(0.1f), color, edgeFactor), 1.0);
}