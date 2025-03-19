#pragma once

#include <iostream>
#include <webgpu/webgpu.h>
#include <glfw3webgpu.h>
#include <window.hpp>
#include <vector>

class renderer
{
private:
    window m_screen;
    WGPUPowerPreference m_powerPreference;

    WGPUInstance m_instance;
    WGPUSurface m_surface;
    WGPUAdapter m_adapter;
    std::vector<WGPUFeatureName> m_adapterFeatures;
    WGPUAdapterProperties m_properties{};
    WGPUDevice m_device;
    std::vector<WGPUFeatureName> m_deviceFeatures;
    WGPUSupportedLimits m_limits{};
    WGPUQueue m_queue;
    WGPUCommandEncoder m_encoder;
    WGPUCommandBuffer m_cmdBuffer;
    WGPUTextureFormat m_format = WGPUTextureFormat_Undefined;
    WGPUTextureView m_targetView;
    WGPURenderPipeline m_pipeline;

public:
    renderer(WGPUPowerPreference preference);
    void createInstance();
    WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const *options);
    void getAdapter();
    void getFeaturesAndProperties();
    WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const *descriptor);
    void getDevice();
    void inspectDevice();
    void createQueue();
    void createCommandEncoder();
    void configureSurface(); // the surface was created when request adapter method ran this method just configures the surface as per the glfw window
    WGPUTextureView getNextsurfaceTextureView();
    void renderPass();
    void createRenderPipeline();
    void run();
    bool isRunning();
    ~renderer();
};
