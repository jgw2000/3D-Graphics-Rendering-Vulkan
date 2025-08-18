
float2 positions[3] = {
    0.0, -0.5,
    -0.5, 0.5,
    0.5, 0.5
};

[shader("vertex")]
float4 vertexMain(uint vertexID : SV_VertexID) : SV_Position
{
    return float4(positions[vertexID], 0.0, 1.0);
}

[shader("fragment")]
float4 fragmentMain()
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
