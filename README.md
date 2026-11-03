# 基于Vulkan从零开发的简单渲染引擎

## 实现的功能
- CSM 级联阴影
- SSAO 屏幕空间遮蔽
- Frustum culling 视锥体剔除


![动画演示效果](./assets/CesiumMan.gif)

[![优化大场景下的CSM阴影](./assets/CSM.png)](https://www.bilibili.com/video/BV1vney6TE5a/)



首次构建需要执行的命令有
``` 

git clone --depth 1 --branch  v2.1         https://github.com/danfis/libccd.git                                  external/libccd
git clone --depth 1 --branch  v1.3.2       https://github.com/USCiLab/cereal.git                                 external/cereal
git clone --depth 1 --branch  v1.17.0      https://github.com/google/googletest.git                              external/googletest
git clone --depth 1 --branch  v3.1.0       https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git external/VulkanMemoryAllocator
git clone --depth 1 --branch  v3.16.0      https://github.com/skypjack/entt.git                                  external/entt
git clone --depth 1 --branch  v11.0.2      https://github.com/odygrd/quill.git                                   external/quill
git clone --depth 1 --branch  v1.13        https://github.com/Chlumsky/msdfgen.git                               external/msdfgen
git clone --depth 1 --branch  v1.1         https://github.com/zeux/meshoptimizer.git                             external/meshoptimizer
git clone --depth 1 --branch  v4.4.2       https://github.com/KhronosGroup/KTX-Software.git                      external/ktx
git clone --depth 1 --branch  v3.3.1       https://github.com/jarro2783/cxxopts.git                              external/cxxopts
git clone --depth 1 --branch  v1.0.7       https://github.com/cameron314/readerwriterqueue.git                   external/readerwriterqueue
git clone --depth 1 --branch  20260817.0   https://github.com/abseil/abseil-cpp.git                              external/absl
wget -P external/tinyobj/  https://github.com/tinyobjloader/tinyobjloader/releases/download/v1.0.6/tiny_obj_loader.h 
```

树莓派上运行需要提前安装的程序
```
sudo apt install -y libfreetype6-dev 
sudo apt install -y libtinyxml2-dev 
sudo apt install -y libeigen3-dev 
sudo apt install vulkan-tools
sudo apt install spirv-cross libspirv-cross-c-shared-dev -y
sudo apt install libopenvdb-dev -y
sudo apt install libboost-iostreams-dev libboost-system-dev -y
sudo apt install -y build-essential cmake libasound2-dev libpulse-dev \
    libwayland-dev wayland-protocols libx11-dev libxrandr-dev \
    libxinerama-dev libxcursor-dev libxi-dev libxss-dev libkbproto-dev \
    libdrm-dev libgbm-dev
sudo apt install libncurses5-dev libncursesw5-dev
sudo apt install libvulkan-dev vulkan-tools libshaderc-dev
sudo apt install -y libnanovdb-dev libnanovdb-tools
sudo apt install libharfbuzz-dev
sudo apt install glslang-tools glslang-dev

```

树莓派上还需要自己下载编译 SDL3
```
git clone --depth 1 --branch release-3.4.4 https://github.com/libsdl-org/SDL.git 
mkdir  build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSDL_X11_XSCRNSAVER=OFF  -DSDL_X11_XTEST=OFF
cmake --build . --config Release -j$(nproc)
sudo cmake --install .

```