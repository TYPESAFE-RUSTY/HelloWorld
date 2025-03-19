## Description

This repository contains code examples demonstrating how to render a triangle using various graphics APIs.

### Branch Structure
- **Main Branch:** Contains only the initialization steps for `GLFW` and `GLM`.
- **Branches Marked "Done":** These branches include "Hello World" examples for specific graphics APIs.

## Getting Started

To clone this repository, run the following command:

```bash
git clone https://github.com/TYPESAFE-RUSTY/HelloWorld.git --recursive
```

> **Note:** The `--recursive` flag is important to ensure all submodules are downloaded.

## Build Instructions

### Option 1: Using Visual Studio
1. Open the project in **Visual Studio**.
2. Select `main.exe` from the build targets.
3. Compile and run the project.

### Option 2: Using Command Line
Ensure you have the following tools installed:
- **Clang**
- **CMake**
- **Make**
- **Ninja**

To build and run the project:

```bash
make
```

## Todo
- [x] opengl
- [x] vulkan
- [x] webgpu
- [ ] directx
- [ ] webgpu (js-example)
