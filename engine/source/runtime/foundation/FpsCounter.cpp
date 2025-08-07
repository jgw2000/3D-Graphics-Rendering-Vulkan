#include "FpsCounter.h"

namespace jgw
{
    bool FpsCounter::Tick(float delta)
    {
        ++numFrames;
        elapsedTime += delta;

        if (elapsedTime > updateInterval)
        {
            currentFPS = static_cast<float>(numFrames / elapsedTime);
            numFrames = 0;
            elapsedTime = 0.0f;
            return true;
        }

        return false;
    }
}
