#include "include/renderer.hpp"

int main()
{
    // renderer takes a power preference convenient way to switch between gpus
    renderer renderer(WGPUPowerPreference_HighPerformance);
    while (renderer.isRunning())
    {
        renderer.run();
    }

    return EXIT_SUCCESS;
}