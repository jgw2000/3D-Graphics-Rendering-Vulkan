struct VSInput
{
    float3 pos;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos;
};

struct HSOutput
{
    float4 pos : SV_Position;
    float3 worldPos;
};

struct DSOutput
{
    float4 pos : SV_Position;
    float3 worldPos;
};

struct ConstantsHSOutput
{
    float TessLevelOuter[3] : SV_TessFactor;
    float TessLevelInner[2] : SV_InsideTessFactor;
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
    float4 cameraPos;
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
    output.worldPos = mul(pcData.model, float4(input.pos, 1.0)).xyz;
    output.pos = mul(pcData.proj, mul(pcData.view, float4(output.worldPos, 1.0)));
    return output;
}

float getTessLevel(float distance0, float distance1)
{
    const float distanceScale1 = 1.2;
    const float distanceScale2 = 1.7;
    const float avgDistance = (distance0 + distance1) / (2.0 * 0.8f);

    if (avgDistance <= distanceScale1) return 5.0;
    if (avgDistance <= distanceScale2) return 3.0;

    return 1.0;
}

ConstantsHSOutput ConstantsHS(InputPatch<VSOutput, 3> patch)
{
    float3 c = pcData.cameraPos.xyz;

    float eyeToVertexDistance0 = distance(c, patch[0].worldPos);
    float eyeToVertexDistance1 = distance(c, patch[1].worldPos);
    float eyeToVertexDistance2 = distance(c, patch[2].worldPos);

    ConstantsHSOutput output;
    output.TessLevelOuter[0] = getTessLevel(eyeToVertexDistance1, eyeToVertexDistance2);
    output.TessLevelOuter[1] = getTessLevel(eyeToVertexDistance2, eyeToVertexDistance0);
    output.TessLevelOuter[2] = getTessLevel(eyeToVertexDistance0, eyeToVertexDistance1);
    output.TessLevelInner[0] = output.TessLevelOuter[2];
    return output;
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHS")]
HSOutput hullMain(InputPatch<VSOutput, 3> patch, uint InvocationID : SV_OutputControlPointID)
{
    HSOutput output;
    output.pos = patch[InvocationID].pos;
    output.worldPos = patch[InvocationID].worldPos;
    return output;
}

[shader("domain")]
[domain("tri")]
DSOutput domainMain(ConstantsHSOutput input, float3 TessCoord: SV_DomainLocation, const OutputPatch<HSOutput, 3> patch)
{
    DSOutput output;
    output.pos = patch[0].pos * TessCoord.x + patch[1].pos * TessCoord.y + patch[2].pos * TessCoord.z;
    output.worldPos = patch[0].worldPos * TessCoord.x + patch[1].worldPos * TessCoord.y + patch[2].worldPos * TessCoord.z;
    return output;
}


[shader("geometry")]
[maxvertexcount(3)]
void geometryMain(triangle DSOutput input[3], inout TriangleStream<GSOutput> outStream)
{
    for (int i = 0; i < 3; i++)
    {
        GSOutput output;
        output.pos = input[i].pos;
        output.color = input[i].worldPos;
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
