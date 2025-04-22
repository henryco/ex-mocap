//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_STATE_H
#define EX_MOTION_STATE_H

#include <CL/cl.h>
#include <GLFW/glfw3.h>

#include <cstdint>

namespace ex {

    struct project_state {
        const char *project_path  = nullptr;
        const char *title         = nullptr;
        bool        has_changes   = false;
        bool        brand_new     = false;
        bool        should_save   = false;
        bool        should_open   = false;
        bool        should_create = false;
    };

    struct error_state {
        const char *msg  = nullptr;
        uint64_t    code = 0;
    };

    struct cl_state {
        cl_context   context = nullptr;
        cl_device_id device  = nullptr;
    };

    struct gl_state {
        GLFWwindow *window     = nullptr;
        int32_t     target_fps = -1;
        bool        sharing    = false;
    };

    struct logic_state {
        uint32_t root_node   = -1;
        uint32_t active_node = -1;
        int16_t  clock_freq  = -1;
    };

    struct metrics_state {
        float frame_duration_ns = 0.0f;
        float logic_duration_ns = 0.0f;
        float total_duration_ns = 0.0f;
        bool  timing_enabled    = true;
    };

    struct State {
        project_state project = {};
        metrics_state metrics = {};
        error_state   error   = {};
        cl_state      cl      = {};
        gl_state      gl      = {};
        logic_state   logic   = {};
    };

}

#endif // EX_MOTION_STATE_H
