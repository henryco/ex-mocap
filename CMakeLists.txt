cmake_minimum_required(VERSION 3.30)
project(ex_motion VERSION 0.2.2 LANGUAGES CXX)

#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -funroll-loops -DNDEBUG" CACHE STRING "Release flags" FORCE)
#set(CMAKE_C_FLAGS_RELEASE   "-O3 -flto -funroll-loops -DNDEBUG" CACHE STRING "Release flags" FORCE)

option(ENABLE_TESTS "Build with unit tests" OFF)
option(ENABLE_VALGRIND "Build with valgrind" OFF)

if (ENABLE_TESTS)
    add_subdirectory(external/doctest)
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ABSL_PROPAGATE_CXX_STD ON)

set(CL_HPP_TARGET_OPENCL_VERSION 300)
set(CL_TARGET_OPENCL_VERSION 300)

find_package(CUDAToolkit 12 REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

set(ONNXRUNTIME_ROOT external/onnxruntime-gpu-cuda12)
include_directories(${ONNXRUNTIME_ROOT}/include)
link_directories(${ONNXRUNTIME_ROOT}/lib)

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

add_library(simpfp INTERFACE)
target_include_directories(simpfp INTERFACE external/simpfp/include)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE external/stb)

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

set(EXTRA_FILES
        # byte box
        extra/bytebox/byte_box.h
        extra/bytebox/byte_box.cpp

        # undo history
        extra/undo/undo_history.h

        # graph (nodes)
        extra/graph/graph.h

        # data structures
        extra/data/data_utils.h
        extra/data/struct/spsc_ring.h
        extra/data/struct/lifo_queue.h
        extra/data/struct/sorted_map.h
        extra/data/struct/array_map.h
        extra/data/struct/list_map.h
        extra/data/struct/rmq_map.h
        extra/data/struct/rb_map.h
        extra/data/struct/buffer.h
        extra/data/struct/cache.h
        extra/data/struct/array.h

        # mem allocators
        extra/data/alloc/datalloc.h
        extra/data/alloc/stackarena.h
        extra/data/alloc/rbtalloc.h
        extra/data/alloc/provider.h
        extra/data/alloc/lalloc.h
        extra/data/struct/byte_array.h
        extra/worker/worker.h
        extra/worker/worker.cpp
        extra/data/struct/dcas_buffer.h
        extra/data/struct/spinlock.h
)

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
        kernels/transform.h
)

set(HEADER_FILES
        src/headers/state.h
        src/headers/boot.h

        src/headers/imgui/gui.h
        src/headers/utils/glfw_utils.h
        src/headers/utils/log_utils.h
        src/headers/utils/gui_utils.h
        src/headers/cl/cl_gl_interop.h
        src/headers/cl/cl_kernel.h
        src/headers/imgui/layout/gui_layout.h
        src/headers/logic/logic.h
        src/headers/logic/logic_nodes.h
        src/headers/imgui/gui_nodes.h
        src/headers/imgui/gui_state.h
        src/headers/imgui/tree/gui_tree_ops.h
        src/headers/imgui/gui_globals.h
        src/headers/logic/logic_state.h
        src/headers/io/byte_serializer.h
        src/headers/utils/file_utils.h
        src/headers/logic/logic_globals.h
        src/headers/imgui/tree/gui_tree_state.h
        src/headers/imgui/tree/gui_tree_menu.h
        src/headers/imgui/strings/gui_strings.h
        src/headers/imgui/actions/gui_actions_registry.h
        src/headers/imgui/actions/gui_actions_keymap.h
        src/headers/imgui/actions/gui_actions.h
        src/headers/utils/time_utils.h
        src/headers/imgui/layout/pane/gui_pane_left.h
        src/headers/imgui/layout/pane/gui_pane_right.h
        src/headers/imgui/layout/pane/gui_pane.h
        src/headers/imgui/display/gui_display.h
        src/headers/io/display_out.h
        src/headers/imgui/display/imp.h
        src/headers/logic/logic_mappers.h
        src/headers/imgui/gui_mappers.h
        src/headers/io/serializer.h
        src/headers/utils/err_utils.h
        src/headers/cl/cl_image.h
        src/headers/entity.h
        src/headers/nodes/node_registry.h
        src/headers/nodes/components/image/image_gui.h
        src/headers/nodes/components/image/image_logic.h
        src/headers/nodes/outputs/display/display_gui.h
        src/headers/nodes/outputs/display/display_logic.h
        src/headers/nodes/components/camera/camera_gui.h
        src/headers/nodes/components/camera/camera_logic.h
        src/headers/nodes/components/transform/transform_logic.h
        src/headers/nodes/components/transform/transform_gui.h
)

set(SOURCE_FILES
        src/sources/imgui/gui.cpp

        src/sources/boot_gui.cpp
        src/sources/boot_terminal.cpp
        src/sources/imgui/layout/gui_layout.cpp
        src/sources/utils/glfw_utils.cpp
        src/sources/utils/log_utils.cpp
        src/sources/utils/gui_utils.cpp
        src/sources/cl/cl_gl_interop.cpp
        src/sources/cl/cl_kernel.cpp
        src/sources/imgui/layout/gui_layout_menu_top.cpp
        src/sources/imgui/layout/gui_layout_logs.cpp
        src/sources/imgui/layout/gui_layout_work_tree.cpp
        src/sources/imgui/layout/gui_layout_history.cpp
        src/sources/imgui/layout/gui_layout_main_content.cpp
        src/sources/logic/logic.cpp
        src/sources/imgui/tree/gui_tree_menu.cpp
        src/sources/imgui/layout/gui_layout_work_window.cpp
        src/sources/imgui/layout/gui_layout_node_inspector.cpp
        src/sources/imgui/gui_globals.cpp
        src/sources/imgui/tree/gui_tree_ops.cpp
        src/sources/logic/logic_state.cpp
        src/sources/io/byte_serializer.cpp
        src/sources/utils/file_utils.cpp
        src/sources/logic/logic_globals.cpp
        src/sources/imgui/tree/gui_tree_state.cpp
        src/sources/imgui/layout/gui_layout_file_dialogs.cpp
        src/sources/imgui/actions/gui_actions_registry.cpp
        src/sources/imgui/actions/gui_actions.cpp
        src/sources/imgui/layout/gui_layout_alert_dialogs.cpp
        src/sources/utils/time_utils.cpp
        src/sources/imgui/layout/pane/gui_pane_left.cpp
        src/sources/imgui/layout/pane/gui_pane_right.cpp
        src/sources/imgui/display/gui_display.cpp
        src/sources/logic/logic_mappers.cpp
        src/sources/imgui/gui_mappers.cpp
        src/sources/utils/err_utils.cpp
        src/sources/cl/cl_image.cpp
        src/sources/entity.cpp
        src/sources/nodes/provider.cpp
        src/sources/nodes/outputs/registry.cpp
        src/sources/nodes/components/registry.cpp
        src/sources/nodes/components/image/image_gui.cpp
        src/sources/nodes/components/image/image_logic.cpp
        src/sources/nodes/outputs/display/display_gui.cpp
        src/sources/nodes/outputs/display/display_logic.cpp
        src/sources/nodes/components/camera/camera_gui.cpp
        src/sources/nodes/components/camera/camera_logic.cpp
        src/sources/nodes/components/transform/transform_gui.cpp
        src/sources/nodes/components/transform/transform_logic.cpp
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
# ===================================================================
add_library(cmake_generated INTERFACE)
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/exmotion_files/generated/include/ex_version.h
)

target_include_directories(cmake_generated INTERFACE
        ${CMAKE_BINARY_DIR}/exmotion_files/generated/include)
# ===================================================================

set(COMP_TARGET)

set(INCL_TARGET
        ${OpenCV_INCLUDE_DIRS})

set(LINK_TARGET
        ${OpenCV_LIBS}
        ${CUDA_LIBRARIES}
        cmake_generated
        spdlog::spdlog
        OpenCL::Headers
        OpenCL::OpenCL
        onnxruntime
        OpenGL::GL
        simpfp
        lucide
        ifcpph
        imgui
        glm
        stb)

set(DEFN_TARGET
        CL_TARGET_OPENCL_VERSION=300
        CL_HPP_TARGET_OPENCL_VERSION=300)

if (ENABLE_VALGRIND)
    list(APPEND DEFN_TARGET USE_VALGRIND)
    list(APPEND DEFN_TARGET LALLOC_USE_VALGRIND)
endif()

if (UNIX AND NOT APPLE)
    list(APPEND INCL_TARGET ${V4L2_INCLUDE_DIRS})
    list(APPEND LINK_TARGET ${V4L2_LIBRARIES})
    list(APPEND COMP_TARGET ${V4L2_CFLAGS_OTHER})
endif ()

IF (NOT ENABLE_TESTS)
    list(APPEND COMP_TARGET "-fno-exceptions")
endif()

set(RESULT_SOURCES
        ${KERNEL_FILES}
        ${GENERATED_CL_SOURCES}
        ${PLATFORM_FILES}
        ${HEADER_FILES}
        ${SOURCE_FILES}
        ${EXTRA_FILES}
)

add_executable(ex_motion
        src/main.cpp
        ${RESULT_SOURCES})

add_dependencies(${PROJECT_NAME}
        generate_cl_sources)

target_include_directories(${PROJECT_NAME}
        PRIVATE ${INCL_TARGET})

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${LINK_TARGET})

target_compile_options(${PROJECT_NAME}
        PRIVATE ${COMP_TARGET})

target_compile_definitions(${PROJECT_NAME}
        PRIVATE ${DEFN_TARGET})

# ====================== TESTS (OPTIONAL) ===========================
if (ENABLE_TESTS)
    set(TEST_SOURCES
            src/tests/test_init.cpp
            src/tests/test_io.cpp
            src/tests/test_graph.cpp
            src/tests/test_data.cpp
            src/tests/test_rb_map.cpp
            src/tests/test_rmq_map.cpp
            src/tests/test_rbtalloc.cpp
            src/tests/test_list_map.cpp
            src/tests/test_lalloc.cpp)

    add_executable(tests
            ${RESULT_SOURCES}
            ${TEST_SOURCES})

    add_dependencies(tests
            generate_cl_sources)

    target_include_directories(tests
            PRIVATE ${INCL_TARGET})

    target_link_libraries(tests
            PRIVATE ${LINK_TARGET}
            PRIVATE doctest)

    target_compile_options(tests
            PRIVATE ${COMP_TARGET})

    target_compile_definitions(tests
            PRIVATE UNIT_TEST
            PRIVATE LALLOC_UNIT_TEST
            PRIVATE ${DEFN_TARGET})

    enable_testing()
    add_test(NAME Tests COMMAND tests)
endif ()
# ===================================================================