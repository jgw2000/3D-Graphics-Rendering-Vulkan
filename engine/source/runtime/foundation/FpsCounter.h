#pragma once

#include <cstdint>

namespace jgw
{
    class FpsCounter
    {
    public:
        bool Tick(float delta);

        inline float GetFPS() const { return currentFPS; }

    private:
        float updateInterval = 1.f;
        float currentFPS = 0.0f;
        float elapsedTime = 0.0f;
        uint32_t numFrames = 0;
    };
}
