# 3D-Graphics-Rendering-Vulkan

## 编译环境和依赖
* Windows
* Visual Studio 2022
* Vulkan SDK 1.4.313
* cmake 3.31
* conan 2
* C++ 20
* Python 3.6+

## 第三方库
* glfw 3.4
* spdlog 1.15.3
* assimp 5.4.3
* stb cci.20230920
* imgui 1.92.0
* implot
* ktx 4.3.2
* meshoptimizer 0.23

## 特性
* Vulkan-HPP
* Vma
* Slang
* Dynamic Rendering

## 编译运行
In the project root directory "Open Git Bash here", execute "bash build.sh" and it will generate VS project solution in the build directory.

### Project 0
![](https://github.com/jgw2000/3D-Graphics-Rendering-Vulkan/blob/main/results/project0.png)

### Project 1
![](https://github.com/jgw2000/3D-Graphics-Rendering-Vulkan/blob/main/results/project1.png)

* Rendering ImGui user interface
* Implementing an immediate-mode 3D drawing canvas
* Adding a first-person 3D camera
* Using cube map textures
* Using storage buffer

### Project 2
![](https://github.com/jgw2000/3D-Graphics-Rendering-Vulkan/blob/main/results/project2.png)

* Generating LOD meshes using MeshOptimizer
* Using geometry shader for wireframe rendering
* Using tessellation shader for hardware LOD
* Implementing gpu instancing
* Implementing an infinite grid shader

### Project 3
* Implementing mesh preprocessing and converting pipeline
* Implementing indirect rendering