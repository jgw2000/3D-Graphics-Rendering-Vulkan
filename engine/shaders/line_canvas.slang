
struct VSOutput
{
    float4 pos : SV_Position;
    float4 color;
};

struct Vertex
{
    float4 pos;
    float4 color;
};

struct PushConstantData
{
    float4x4 mvp;
    Vertex *vertices;
};
[[vk::push_constant]] PushConstantData pcData;

[shader("vertex")]
VSOutput vertexMain(uint vertexID : SV_VertexID)
{
    VSOutput output;
    output.pos = pcData.vertices[vertexID].pos;
    output.color = pcData.vertices[vertexID].color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    return input.color;
}