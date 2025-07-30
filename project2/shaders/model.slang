[shader("vertex")]
float4 vertexMain(float3 position : POSITION0) : SV_Position
{
    return float4(position, 1.0);
}

[shader("fragment")]
float4 fragmentMain()
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
