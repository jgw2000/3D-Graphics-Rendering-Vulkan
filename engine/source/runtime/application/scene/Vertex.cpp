#include "Vertex.h"

namespace jgw
{
    uint32_t VertexInput::GetAttributeNum() const
    {
        uint32_t n = 0;
        while (n < kMaxVertexAttributes && attributes[n].format != vk::Format::eUndefined)
            ++n;
        return n;
    }

    uint32_t VertexInput::GetInputBindingNum() const
    {
        uint32_t n = 0;
        while (n < kMaxVertexBuffers && inputBindings[n].stride)
            ++n;
        return n;
    }

    uint32_t VertexInput::GetVertexSize() const
    {
        uint32_t vertexSize = 0;
        for (uint32_t i = 0; i < kMaxVertexAttributes && attributes[i].format != vk::Format::eUndefined; ++i)
            vertexSize += GetVertexFormatSize(attributes[i].format);
        return vertexSize;
    }

    uint32_t GetVertexFormatSize(vk::Format format)
    {
        return 0;
    }
}