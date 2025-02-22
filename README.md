### How to clone properly
```shell
    git clone \
        --recurse-submodules \
        git@github.com:henryco/ex-mocap.git
```

### Dependencies
- [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders) (external/iconfontcppheaders)
- [LucideIcons](https://github.com/lucide-icons/lucide) (external/lucide)
- [OnnxRuntime](https://github.com/microsoft/onnxruntime) (external/onnxruntime)
- [Doctest](https://github.com/doctest/doctest) (external/doctest)
- [ImGui](https://github.com/ocornut/imgui) (external/imgui)
- [GLM](https://github.com/g-truc/glm) (external/glm)
- [Spdlog](https://github.com/gabime/spdlog)
- [OpenCV](https://opencv.org/get-started)
- [OpenCL](https://www.khronos.org/opencl/)
- [OpenGL](https://www.opengl.org/)
- [GLFW](https://www.glfw.org/)
- [GLEW](https://glew.sourceforge.net/)
- [CUDA](https://developer.nvidia.com/cuda-toolkit)
- [V4L2 (linux)](https://trac.gateworks.com/wiki/linux/v4l2)

#### How to install dependencies (Debian/Nvidia)
```shell
    apt install -y          \
        opencl-headers      \
        ocl-icd-libopencl1  \
        nvidia-opencl-dev   \
        libopencv-dev       \
        libspdlog-dev       \
        nvidia-cuda-toolkit \
        cudnn-cuda-12       \
        libglfw3-dev        \
        libglfw3            \
        libglew-dev         \
        libv4l-dev
```
