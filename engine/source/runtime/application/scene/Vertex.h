#pragma once

#include "Common.h"

namespace jgw
{
    const uint32_t kMaxLODs = 7;
    const uint32_t kMaxVertexAttributes = 16;
    const uint32_t kMaxVertexBuffers = 16;

    struct VertexAttribute final
    {
        uint32_t location = 0;
        uint32_t binding = 0;
        vk::Format format = vk::Format::eUndefined;
        uintptr_t offset = 0;
    };

    struct VertexInputBinding final
    {
        uint32_t stride = 0;
    };

    struct VertexInput final
    {
        VertexAttribute attributes[kMaxVertexAttributes];
        VertexInputBinding inputBindings[kMaxVertexBuffers];

        uint32_t GetAttributeNum() const;
        uint32_t GetInputBindingNum() const;
        uint32_t GetVertexSize() const;

        bool operator==(const VertexInput& other) const
        {
            return memcmp(this, &other, sizeof(VertexInput)) == 0;
        }
    };

    uint32_t GetVertexFormatSize(vk::Format format);

    // Use to write values into MeshData::vertexData
    template <typename T>
    inline void Put(std::vector<uint8_t>& v, const T& value)
    {
        const size_t pos = v.size();
        v.resize(v.size() + sizeof(value));
        memcpy(v.data() + pos, &value, sizeof(value));
    }
}