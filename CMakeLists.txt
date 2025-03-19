cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

# gives name to the output executable file 
project(main)

message("Adding all cpp files in src folder as dependencies")
file(GLOB SOURCES src/*.cpp)

message("Adding submodules as dependencies")
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_subdirectory(glfw3webgpu)
add_subdirectory(webgpu)

# adds all the cpp files (check previous file() to get more info) as deps when compiling 
# e.g clang a.cpp b.cpp c.cpp -o main.exe

add_executable(${PROJECT_NAME} ${SOURCES})

message("Updating include path")
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/src/include
)

message("Linking command updated")
target_link_libraries(${PROJECT_NAME} glfw webgpu glfw3webgpu)

message("All ready to compile")

# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it (it's called WGPU_RUNTIME_LIB in general)
# next to the binary.
# does nothing when using dawn as the backend 
# works only with wgpu-native as it contains pre compiled binaries.
target_copy_webgpu_binaries(main)