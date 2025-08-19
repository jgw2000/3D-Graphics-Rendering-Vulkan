// extents of grid in world coordinates
static float gridSize = 100.0;

// size of one cell
static float gridCellSize = 0.025;

// color of thin lines
static float4 gridColorThin = float4(0.5, 0.5, 0.5, 1.0);

// color of thick lines (every tenth line)
static float4 gridColorThick = float4(0.0, 0.0, 0.0, 1.0);

// minimum number of pixels between cell lines before LOD switch should occur.
static float gridMinPixelsBetweenCells = 2.0;

static float3 pos[4] =
{
    -1.0, 0.0, -1.0,
    -1.0, 0.0, 1.0,
    1.0, 0.0, 1.0,
    1.0, 0.0, -1.0
};

static uint indices[6] =
{
    0, 1, 2, 2, 3, 0
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 camPos;
    float2 uv;
};

struct PushConstantData
{
    float4x4 mvp;
    float4 cameraPos;
};
[[vk::push_constant]] PushConstantData pcData;

float max2(float2 v)
{
    return max(v.x, v.y);
}

float satf(float x)
{
    return clamp(x, 0.0, 1.0);
}

float2 satv(float2 x)
{
    return clamp(x, float2(0.0), float2(1.0));
}

// ref: https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8
float4 gridColor(float2 uv, float2 camPos)
{
    float2 dudv = float2(
        length(float2(ddx(uv.x), ddy(uv.x))),
        length(float2(ddx(uv.y), ddy(uv.y)))
    );

    float lodLevel = max(0.0, log10(length(dudv) * gridMinPixelsBetweenCells / gridCellSize) + 1.0);
    float lodFade = fract(lodLevel);

    // cell sizes for lod0, lod1 and lod2
    float lod0 = gridCellSize * pow(10.0, floor(lodLevel));
    float lod1 = lod0 * 10.0;
    float lod2 = lod1 * 10.0;

    // each anti-aliased line covers up to 4 pixels
    dudv *= 4.0;

    // set grid coordinates to the centers of anti-aliased lines for subsequent alpha calculations
    uv += dudv * 0.5;

    // calculate absolute distances to cell line centers for each lod and pick max X/Y to get coverage alpha value
    float lod0a = max2(float2(1.0) - abs(satv(abs(uv % lod0) / dudv) * 2.0 - float2(1.0)));
    float lod1a = max2(float2(1.0) - abs(satv(abs(uv % lod1) / dudv) * 2.0 - float2(1.0)));
    float lod2a = max2(float2(1.0) - abs(satv(abs(uv % lod2) / dudv) * 2.0 - float2(1.0)));

    uv -= camPos;

    // blend between falloff colors to handle LOD transition
    float4 c = lod2a > 0.0 ? gridColorThick : lod1a > 0.0 ? lerp(gridColorThick, gridColorThin, lodFade) : gridColorThin;

    // calculate opactiy falloff based on distance to grid extents
    float opacityFalloff = (1.0 - satf(length(uv) / gridSize));

    // blend between LOD level alphas and scale with opacity falloff
    c.w *= (lod2a > 0.0 ? lod2a : lod1a > 0.0 ? lod1a : (lod0a * (1.0 - lodFade))) * opacityFalloff;

    return c;
}

[shader("vertex")]
VSOutput vertexMain(uint vertexID: SV_VertexID)
{
    uint idx = indices[vertexID];
    float3 position = pos[idx] * gridSize;
    position.x += pcData.cameraPos.x;
    position.z += pcData.cameraPos.z;

    VSOutput output;
    output.pos = mul(pcData.mvp, float4(position, 1.0));
    output.camPos = pcData.cameraPos.xz;
    output.uv = position.xz;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    return gridColor(input.uv, input.camPos);
}