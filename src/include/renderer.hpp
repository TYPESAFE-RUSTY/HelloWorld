#pragma Once

#define GLFW_INCLUDE_VULKAN
#define NOMINMAX // max macro was clashing with std::max()

#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <algorithm> // Necessary for std::clamp
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <GLFW/glfw3.h>
#include <termcolor/termcolor.hpp>
#include <optional>

const auto DISABLE_LOGS = false;

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class renderer
{
public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif // NDEBUG

  void run();
  ~renderer();

private:
  GLFWwindow *m_window;
  VkInstance m_instance;
  VkSurfaceKHR m_surface;
  VkDebugUtilsMessengerEXT m_debugMessenger;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device; // this is logical device
  VkQueue m_graphicsQueue;
  VkQueue m_presentQueue;
  VkSwapchainKHR m_swapChain;
  std::vector<VkImage> m_swapChainImages;
  VkFormat m_swapChainImageFormat;
  VkExtent2D m_swapChainExtent;
  std::vector<VkImageView> m_swapChainImageViews;
  VkRenderPass m_renderPass;
  VkPipelineLayout m_pipelineLayout;
  VkPipeline m_graphicsPipeline;
  std::vector<VkFramebuffer> m_swapChainFramebuffers;
  VkCommandPool m_commandPool;
  VkCommandBuffer m_commandBuffer;
  const std::vector<const char *> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  // syncronization objects
  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

  void initWindow();
  void initVulkan();
  void mainLoop();
  void createInstance();
  void createSurface();
  void setupDebugMessenger();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();
  void createImageViews();
  void createRenderPass();
  void createGraphicsPipeline();
  void createFramebuffers();
  void createCommandPool();
  void createCommandBuffer();

  void drawFrame();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void createSyncObjects();

  // helpers
  VkShaderModule createShaderModule(const std::vector<char> &code);
  static std::vector<char> readFile(const std::string &filename);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  bool checkDeviceExtesnsionSupport(VkPhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  int rateDeviceSuitablity(VkPhysicalDevice device);
  bool isDeviceSuitable(VkPhysicalDevice device);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  bool checkValidationLayerSupport();
  std::vector<const char *> getRequiredExtensions();
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
      void *pUserData);
};

// below 2 functions are proxy functions that create and destroy debug utils
inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    func(instance, debugMessenger, pAllocator);
  }
}