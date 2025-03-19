cmake_minimum_required(VERSION 3.30)
set(CMAKE_PREFIX_PATH "D:/SDKs/Vulkan") #no need to add this (this line is here to modify the default folder to look for vulkan sdk)
find_package(Vulkan REQUIRED)
set(CMAKE_CXX_STANDARD 17)

# gives name to the output executable file 
project(main)

message("Adding all cpp files in src folder as dependencies")
file(GLOB SOURCES src/*.cpp)

message("Adding submodules as dependencies")
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)

add_executable(${PROJECT_NAME} ${SOURCES})

message("Updating include path")
target_include_directories(${PROJECT_NAME}
    PUBLIC ${Vulkan_INCLUDE_DIRS}
    PUBLIC ${PROJECT_SOURCE_DIR}/src/include
    PUBLIC ${PROJECT_SOURCE_DIR}/lib/glfw/include
    PUBLIC ${PROJECT_SOURCE_DIR}/lib/glm/glm
    PUBLIC ${PROJECT_SOURCE_DIR}/lib/termcolor/include
)

message("Updating Linkers path")
target_link_directories(${PROJECT_NAME}
    PRIVATE lib/glfw/src
)

message("Linking command updated")
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})

message("All ready to compile")