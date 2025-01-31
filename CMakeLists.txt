cmake_minimum_required(VERSION 3.30)
project(ex_motion VERSION 0.1 LANGUAGES CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ABSL_PROPAGATE_CXX_STD ON)
set(JSON_BuildTests OFF CACHE INTERNAL "")

set(CL_HPP_TARGET_OPENCL_VERSION 300)
set(CL_TARGET_OPENCL_VERSION 300)

find_package(CUDAToolkit 12 REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

set(ONNXRUNTIME_ROOT external/onnxruntime-gpu-cuda12)
include_directories(${ONNXRUNTIME_ROOT}/include)
link_directories(${ONNXRUNTIME_ROOT}/lib)

add_subdirectory(external/argparse)
add_subdirectory(external/json)
add_subdirectory(external/glm)

set(CMAKE_C_COMPILER "clang-18")
set(CMAKE_CXX_COMPILER "clang++-18")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
find_package(OpenCLHeaders REQUIRED)
find_package(OpenCL 3.0 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)

if (OpenCL_FOUND)
    add_definitions("-DCL_HPP_TARGET_OPENCL_VERSION=300")
    add_definitions("-DCL_TARGET_OPENCL_VERSION=300")
endif ()

add_library(ifcpph INTERFACE)
target_include_directories(ifcpph INTERFACE external/iconfontcppheaders)

add_library(lucide INTERFACE)
target_include_directories(lucide INTERFACE external/lucide)

add_library(imgui
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/misc/cpp/imgui_stdlib.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
        external/imgui/backends/imgui_impl_glfw.cpp)

target_include_directories(imgui PUBLIC
        external/imgui
        external/imgui/misc/cpp
        external/imgui/backends)

target_link_libraries(imgui PUBLIC
        OpenGL::GL glfw)

set(PLATFORM_FILES
        platforms/device.h
)

if (UNIX AND NOT APPLE)
    pkg_check_modules(V4L2 REQUIRED libv4l2)
    list(APPEND PLATFORM_FILES platforms/linux/v4l2_device.h)
    list(APPEND PLATFORM_FILES platforms/linux/v4l2_device.cpp)
endif ()

set(KERNEL_FILES
        kernels/subsense.h
        kernels/chroma_key.h
        kernels/flip_rotate.h
        kernels/color_space.h
        kernels/filter_conv.h
        kernels/letter_box.h
)

set(HEADER_FILES
        src/headers/state.h
        src/headers/boot.h

        src/headers/imgui/gui.h
        src/headers/utils/gl_utils.h
        src/headers/utils/log_utils.h
        src/headers/utils/gui_utils.h
        src/headers/cl/cl_gl_interop.h
        src/headers/cl/cl_kernel.h
        src/headers/cl/ocl_container.h
        src/headers/cl/ocl_data.h
        src/headers/cl/ocl_interop.h
        src/headers/cl/ocl_interop_ext.h
        src/headers/imgui/gui_layout.h
        src/headers/nodes/nodes.h
        src/headers/nodes/test_nodes.h
        src/headers/core/camera.h
        src/headers/components/c_camera.h
        src/headers/logic.h
        src/headers/nodes/nodes_component.h
        src/headers/components/image_protocol.h
        src/headers/imgui/gui_component.h
        src/headers/imgui/gui_state.h
        src/headers/imgui/cmp/test_cmp.h
        src/headers/undo/undo_history.h
        src/headers/imgui/gui_tree_ops.h
        src/headers/imgui/gui_keymap.h
        src/headers/imgui/gui_globals.h
)

set(SOURCE_FILES
        src/sources/imgui/gui.cpp

        src/sources/boot_gui.cpp
        src/sources/boot_terminal.cpp
        src/sources/imgui/gui_layout.cpp
        src/sources/utils/gl_utils.cpp
        src/sources/utils/log_utils.cpp
        src/sources/utils/gui_utils.cpp
        src/sources/cl/cl_gl_interop.cpp
        src/sources/cl/cl_kernel.cpp
        src/sources/cl/ocl_container.cpp
        src/sources/cl/ocl_data.cpp
        src/sources/cl/ocl_interop.cpp
        src/sources/imgui/layout/gui_layout_menu_top.cpp
        src/sources/imgui/layout/gui_layout_pane_left.cpp
        src/sources/imgui/layout/gui_layout_pane_right.cpp
        src/sources/imgui/layout/gui_layout_logs.cpp
        src/sources/imgui/layout/gui_layout_work_tree.cpp
        src/sources/imgui/layout/gui_layout_history.cpp
        src/sources/imgui/layout/gui_layout_main_content.cpp
        src/sources/core/camera.cpp
        src/sources/components/c_camera.cpp
        src/sources/logic.cpp
        src/sources/state/mappers_camera.cpp
        src/sources/state.cpp
        src/sources/imgui/cmp/test_cmp.cpp
        src/sources/imgui/layout/gui_layout_tree_menu.cpp
        src/sources/imgui/layout/gui_layout_work_window.cpp
        src/sources/imgui/layout/gui_layout_node_inspector.cpp
        src/sources/imgui/gui_globals.cpp
        src/sources/imgui/gui_tree_ops.cpp
)

# =================== OpenCL kernels generation =====================
add_executable(convert_to_c_array scripts/embed_file.cxx)
set(GENERATED_CL_SOURCES)

foreach (input_file IN LISTS KERNEL_FILES)
    get_filename_component(output_path ${input_file} DIRECTORY)
    get_filename_component(output_name ${input_file} NAME_WE)
    set(kernel_name "${PROJECT_SOURCE_DIR}/${output_path}/cl/${output_name}.cl")
    set(output_name "${PROJECT_BINARY_DIR}/exmotion_files/generated/${output_path}/${output_name}.cpp")
    add_custom_command(
            OUTPUT ${output_name}
            COMMAND convert_to_c_array ${PROJECT_SOURCE_DIR}/${input_file} ${kernel_name} ${output_name}
            DEPENDS convert_to_c_array ${PROJECT_SOURCE_DIR}/${input_file} ${kernel_name}
            COMMENT "Converting: ${input_file} -> ${output_name}"
            VERBATIM)
    list(APPEND GENERATED_CL_SOURCES ${output_name})
endforeach ()

add_custom_target(generate_cl_sources
        ALL DEPENDS ${GENERATED_CL_SOURCES})
# =================== OpenCL kernels generation =====================

set(RESULT_SOURCES
        src/main.cpp
        ${KERNEL_FILES}
        ${GENERATED_CL_SOURCES}
        ${PLATFORM_FILES}
        ${HEADER_FILES}
        ${SOURCE_FILES}
)

add_executable(ex_motion
        ${RESULT_SOURCES})

add_dependencies(${PROJECT_NAME}
        generate_cl_sources)

if (UNIX AND NOT APPLE)
    target_include_directories(${PROJECT_NAME}
            PRIVATE ${OpenCV_INCLUDE_DIRS}
            PRIVATE ${V4L2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}
            PRIVATE onnxruntime
            PRIVATE ${CUDA_LIBRARIES}
            PRIVATE ${OpenCV_LIBS}
            PRIVATE OpenGL::GL
            PRIVATE OpenCL::OpenCL
            PRIVATE OpenCL::Headers
            PRIVATE spdlog::spdlog
            PRIVATE ${V4L2_LIBRARIES}
            PRIVATE nlohmann_json::nlohmann_json
            PRIVATE argparse
            PRIVATE lucide
            PRIVATE ifcpph
            PRIVATE imgui
            PRIVATE glm)
    target_compile_options(${PROJECT_NAME}
            PRIVATE ${V4L2_CFLAGS_OTHER})
endif ()

target_compile_definitions(${PROJECT_NAME}
        PRIVATE CL_TARGET_OPENCL_VERSION=300
        PRIVATE CL_HPP_TARGET_OPENCL_VERSION=300)