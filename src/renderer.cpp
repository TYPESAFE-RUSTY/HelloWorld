#include "renderer.hpp"

void renderer::run()
{
    initWindow();
    initVulkan();
    mainLoop();
}

void renderer::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
}

void renderer::initVulkan()
{
    createInstance();
    createSurface();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
}

void renderer::mainLoop()
{
    while (!glfwWindowShouldClose(this->m_window))
    {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(m_device);
}

void renderer::drawFrame()
{
    vkWaitForFences(m_device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(m_commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(m_commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_presentQueue, &presentInfo);
}

void renderer::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(m_device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

renderer::~renderer()
{
    std::cout << termcolor::on_bright_green << termcolor::green << "cleaning" << termcolor::reset << std::endl;
    vkDestroySemaphore(m_device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(m_device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(m_device, inFlightFence, nullptr);
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    for (auto framebuffer : m_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    for (auto imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
    vkDestroyDevice(m_device, nullptr);
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
    std::cout << termcolor::on_bright_green << termcolor::green << "Cleaning Successful!" << termcolor::reset << std::endl;
}
