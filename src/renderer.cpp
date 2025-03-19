#include <renderer.hpp>
#include <cassert>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__

// We embbed the source of the shader module here
const char *SHADER_SOURCE = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
    var p = vec2f(0.0, 0.0);
    if (in_vertex_index == 0u) {
        p = vec2f(-0.5, -0.5);
    } else if (in_vertex_index == 1u) {
        p = vec2f(0.5, -0.5);
    } else {
        p = vec2f(0.0, 0.5);
    }
    return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.4, 1.0, 1.0);
})";

renderer::renderer(WGPUPowerPreference preference) : m_screen(800, 600, "Hello World!")
{
    this->m_powerPreference = preference;
    this->createInstance();
    this->getAdapter();
    this->getFeaturesAndProperties();
    this->getDevice();
    this->inspectDevice();
    this->createQueue();
    this->configureSurface();
    this->createRenderPipeline();
}

void renderer::createInstance()
{
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
    // Make sure the uncaptured error callback is called as soon as an error
    // occurs rather than at the next call to "wgpuDeviceTick".
    WGPUDawnTogglesDescriptor toggles;
    toggles.chain.next = nullptr;
    toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
    toggles.disabledToggleCount = 0;
    toggles.enabledToggleCount = 1;
    const char *toggleName = "enable_immediate_error_handling";
    toggles.enabledToggles = &toggleName;

    desc.nextInChain = &toggles.chain;
#endif // WEBGPU_BACKEND_DAWN

    this->m_instance = wgpuCreateInstance(&desc);

    if (!this->m_instance)
    {
        fprintf(stderr, "unable to initalize webgpu instance");
    }
}

/**
 * Utility function to get a WebGPU adapter, so that
 *     WGPUAdapter adapter = requestAdapterSync(options);
 * is roughly equivalent to
 *     const adapter = await navigator.gpu.requestAdapter(options);
 */
WGPUAdapter renderer::requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const *options)
{
    // A simple structure holding the local information shared with the
    // onAdapterRequestEnded callback.
    struct UserData
    {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    // Callback called by wgpuInstanceRequestAdapter when the request returns
    // This is a C++ lambda function, but could be any function defined in the
    // global scope. It must be non-capturing (the brackets [] are empty) so
    // that it behaves like a regular C function pointer, which is what
    // wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
    // is to convey what we want to capture through the pUserData pointer,
    // provided as the last argument of wgpuInstanceRequestAdapter and received
    // by the callback as its last argument.
    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *pUserData)
    {
        UserData &userData = *reinterpret_cast<UserData *>(pUserData);
        if (status == WGPURequestAdapterStatus_Success)
        {
            userData.adapter = adapter;
        }
        else
        {
            std::cout << "Could not get WebGPU adapter: " << message << std::endl;
        }
        userData.requestEnded = true;
    };

    // Call to the WebGPU request adapter procedure
    wgpuInstanceRequestAdapter(
        instance /* equivalent of navigator.gpu */,
        options,
        onAdapterRequestEnded,
        (void *)&userData);

// We wait until userData.requestEnded gets true
#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded)
    {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__

    assert(userData.requestEnded);

    return userData.adapter;
}

void renderer::getAdapter()
{
    WGPURequestAdapterOptions adapterOpts{};
    adapterOpts.powerPreference = this->m_powerPreference;
    adapterOpts.nextInChain = nullptr;
    this->m_surface = glfwGetWGPUSurface(this->m_instance, this->m_screen.pubWindow);
    adapterOpts.compatibleSurface = this->m_surface;
    m_adapter = this->requestAdapterSync(this->m_instance, &adapterOpts);
}

void renderer::getFeaturesAndProperties()
{
    size_t featureCount = wgpuAdapterEnumerateFeatures(this->m_adapter, nullptr);
    this->m_adapterFeatures.resize(featureCount);

    wgpuAdapterEnumerateFeatures(this->m_adapter, this->m_adapterFeatures.data());

    this->m_properties.nextInChain = nullptr;
    wgpuAdapterGetProperties(this->m_adapter, &this->m_properties);

    if (this->m_properties.name)
    {
        std::cout << this->m_properties.name << std::endl;
    }
}

/**
 * Utility function to get a WebGPU device, so that
 *     WGPUDevice device = requestDeviceSync(adapter, options);
 * is roughly equivalent to
 *     const device = await adapter.requestDevice(descriptor);
 * It is very similar to requestAdapter
 */
WGPUDevice renderer::requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const *descriptor)
{
    struct UserData
    {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const *message, void *pUserData)
    {
        UserData &userData = *reinterpret_cast<UserData *>(pUserData);
        if (status == WGPURequestDeviceStatus_Success)
        {
            userData.device = device;
        }
        else
        {
            std::cout << "Could not get WebGPU device: " << message << std::endl;
        }
        userData.requestEnded = true;
    };

    wgpuAdapterRequestDevice(
        adapter,
        descriptor,
        onDeviceRequestEnded,
        (void *)&userData);

#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded)
    {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__

    assert(userData.requestEnded);

    return userData.device;
}

void renderer::getDevice()
{
    WGPUDeviceDescriptor descriptor{};
    descriptor.nextInChain = nullptr;
    descriptor.label = this->m_screen.pubTitle.data();
    descriptor.requiredFeatureCount = 0;
    descriptor.requiredLimits = nullptr;
    descriptor.defaultQueue.nextInChain = nullptr;
    descriptor.defaultQueue.label = "default queue";

    // setting up deviceLostCallbackInfo on descriptor
    // this requries us to create a callback with correct signature
    // then construct callbackInfo struct
    // then pass that struct to the descriptor
    WGPUDeviceLostCallbackNew lostCallback = [](const WGPUDevice *device, WGPUDeviceLostReason reason, char const *message, void * /* pUserData */)
    {
        std::cout << "Device lost: reason " << reason;
        if (message)
            std::cout << " (" << message << ")";
        std::cout << std::endl;
    };

    WGPUDeviceLostCallbackInfo lostCallbackInfo{};
    lostCallbackInfo.nextInChain = nullptr;
    lostCallbackInfo.callback = lostCallback;

    descriptor.deviceLostCallbackInfo = lostCallbackInfo;
    this->m_device = this->requestDeviceSync(this->m_adapter, &descriptor);

    // adding a callback to capture errors
    auto onDeviceError = [](WGPUErrorType type, char const *message, void * /* pUserData */)
    {
        std::cout << "Uncaptured device error: type " << type;
        if (message)
            std::cout << " (" << message << ")";
        std::cout << std::endl;
    };
    wgpuDeviceSetUncapturedErrorCallback(this->m_device, onDeviceError, nullptr /* pUserData */);
}

void renderer::inspectDevice()
{
    size_t featureCount = wgpuDeviceEnumerateFeatures(this->m_device, nullptr);
    m_deviceFeatures.resize(featureCount);
    wgpuDeviceEnumerateFeatures(this->m_device, this->m_deviceFeatures.data());

    this->m_limits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
    bool success = wgpuDeviceGetLimits(this->m_device, &this->m_limits) == WGPUStatus_Success;
#else
    bool success = wgpuDeviceGetLimits(this->m_device, &this->m_limits);
#endif

    if (!success)
    {
        fprintf(stderr, "unable to fetch device limits");
    }
}

void renderer::createQueue()
{
    this->m_queue = wgpuDeviceGetQueue(this->m_device);
    auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void * /* pUserData */)
    {
        std::cout << "Queued work finished with status: " << status << std::endl;
    };
    wgpuQueueOnSubmittedWorkDone(this->m_queue, onQueueWorkDone, nullptr /* pUserData */); // i have no idea what to pass as signalvalue parameter
}

void renderer::createCommandEncoder()
{
    WGPUCommandEncoderDescriptor encoderDesc{};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "command encoder";
    this->m_encoder = wgpuDeviceCreateCommandEncoder(this->m_device, &encoderDesc);
}

// the surface was created when request adapter method ran this method just configures the surface as per the glfw window
void renderer::configureSurface()
{
    WGPUSurfaceConfiguration config{};
    config.nextInChain = nullptr;
    config.height = this->m_screen.getHeight();
    config.width = this->m_screen.getWidth();
    WGPUSurfaceCapabilities capablities{};
    wgpuSurfaceGetCapabilities(this->m_surface, this->m_adapter, &capablities);
    this->m_format = capablities.formats[0];
    WGPUTextureFormat surfaceFormat = this->m_format;
    config.format = surfaceFormat;
    // And we do not need any particular view format:
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.device = this->m_device;
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = WGPUCompositeAlphaMode_Auto;

    wgpuSurfaceConfigure(this->m_surface, &config);
} // the surface was created when request adapter method ran this method just configures the surface as per the glfw window

WGPUTextureView renderer::getNextsurfaceTextureView()
{
    // get surface texture
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(this->m_surface, &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success)
    {
        return nullptr;
    }

    // create textureView
    WGPUTextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

    // release surface texture
    wgpuTextureRelease(surfaceTexture.texture);
    return targetView;
}

void renderer::renderPass()
{
    // describe render pass
    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.nextInChain = nullptr;

    WGPURenderPassColorAttachment renderPassColorAttachment = {};
    renderPassColorAttachment.view = this->m_targetView;
    renderPassColorAttachment.resolveTarget = nullptr; // this is null as multisampling is not used currently
    renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{0.0, 0.0, 0.0, 1.0};

#ifndef WEBGPU_BACKEND_WGPU
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(this->m_encoder, &renderPassDesc);
    // use render pass

    wgpuRenderPassEncoderSetPipeline(renderPass, this->m_pipeline);
    wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);

    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);
}

void renderer::createRenderPipeline()
{
    // Load the shader module
    WGPUShaderModuleDescriptor shaderDesc{};
#ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
#endif

    // We use the extension mechanism to specify the WGSL part of the shader module descriptor
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
    // Set the chained struct's header
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    // Connect the chain
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    shaderCodeDesc.code = SHADER_SOURCE;
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(this->m_device, &shaderDesc);

    // Create the render pipeline
    WGPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.nextInChain = nullptr;

    // describe vertex stage
    pipelineDesc.vertex.bufferCount = 0;
    pipelineDesc.vertex.buffers = nullptr;
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;

    // Each sequence of 3 vertices is considered as a triangle
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;

    // describe fragment stage
    WGPUFragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    WGPUBlendState blendState{};
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.color.operation = WGPUBlendOperation_Add;
    blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
    blendState.alpha.dstFactor = WGPUBlendFactor_One;
    blendState.alpha.operation = WGPUBlendOperation_Add;

    WGPUColorTargetState colorTarget{};
    colorTarget.format = this->m_format;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = WGPUColorWriteMask_All; // We could write to only some of the color channels.

    // We have only one target because our render pass has only one output color
    // attachment.
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;

    // We do not use stencil/depth testing for now
    pipelineDesc.depthStencil = nullptr;

    // Samples per pixel
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    pipelineDesc.layout = nullptr;

    this->m_pipeline = wgpuDeviceCreateRenderPipeline(this->m_device, &pipelineDesc);

    // We no longer need to access the shader module
    wgpuShaderModuleRelease(shaderModule);
}

void renderer::run()
{
    glfwPollEvents();
    this->m_targetView = this->getNextsurfaceTextureView();
    if (!this->m_targetView)
        return;
    this->createCommandEncoder();
    this->renderPass();

    // encode and submit the render pass
    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(this->m_encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(this->m_encoder);

    wgpuQueueSubmit(this->m_queue, 1, &command);
    wgpuCommandBufferRelease(command);

    wgpuTextureViewRelease(this->m_targetView);
    wgpuSurfacePresent(this->m_surface);
#if defined(WEBGPU_BACKEND_DAWN)
    wgpuDeviceTick(this->m_device);
#elif defined(WEBGPU_BACKEND_WGPU)
    wgpuDevicePoll(this->m_device, false, nullptr);
#endif
}

bool renderer::isRunning()
{
    return glfwGetKey(this->m_screen.pubWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(this->m_screen.pubWindow) == 0;
}

renderer::~renderer()
{
    wgpuRenderPipelineRelease(this->m_pipeline);
    wgpuSurfaceUnconfigure(this->m_surface);
    wgpuQueueRelease(this->m_queue);
    wgpuSurfaceRelease(this->m_surface);
    wgpuDeviceRelease(this->m_device);
    wgpuAdapterRelease(this->m_adapter);
    wgpuInstanceRelease(this->m_instance);
}
