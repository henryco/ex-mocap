### How to clone properly
```shell
git clone \
    --recurse-submodules \
    git@github.com:henryco/ex-mocap.git
  ```

### Dependencies
- [OnnxRuntime](https://github.com/microsoft/onnxruntime) (external/onnxruntime)
- [Argparse](https://github.com/p-ranav/argparse#positional-arguments) (external/argparse)
- [ImGui](https://github.com/ocornut/imgui) (external/imgui)
- [JSON](https://github.com/nlohmann/json) (external/json)
- [Glm](https://github.com/g-truc/glm) (external/glm)
- [V4l2 (linux)](https://trac.gateworks.com/wiki/linux/v4l2)
- [OpenCV](https://opencv.org/get-started)
- [OpenCL](https://www.khronos.org/opencl/)
- [OpenGL](https://www.opengl.org/)
- [GLFW](https://www.glfw.org/)
- [Spdlog](https://github.com/gabime/spdlog)
- [CUDA](https://developer.nvidia.com/cuda-toolkit)

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
    libv4l-dev
```
