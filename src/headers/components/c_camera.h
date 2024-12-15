//
// Created by xd on 15/12/24.
//

#ifndef C_CAMERA_H
#define C_CAMERA_H

#include "../core/camera.h"
#include "../nodes/nodes.h"

namespace ex::cmp {

    class CameraNode : public nodes::Node<CameraNode>,
                       public nodes::State<camera::Camera>,

                       public nodes::id::name<"camera_node">,

                       public nodes::io::out<xm::ocl::Image2D, 0, "Captured frame">,

                       public nodes::io::in <bool,        1,  "Fast mode">,
                       public nodes::io::in <bool,        2,  "Flip horizontal">,
                       public nodes::io::in <bool,        3,  "Flip vertical">,
                       public nodes::io::in <bool,        4,  "Rotate">,
                       public nodes::io::in <int,         5,  "Width">,
                       public nodes::io::in <int,         6,  "Height">,
                       public nodes::io::in <int,         7,  "Buffer">,
                       public nodes::io::in <int,         8,  "FPS">,
                       public nodes::io::in <int,         9,  "x">,
                       public nodes::io::in <int,         10, "y">,
                       public nodes::io::in <int,         11, "w">,
                       public nodes::io::in <int,         12, "h">,
                       public nodes::io::in <const char*, 13, "codec">,
                       public nodes::io::in <const char*, 14, "uuid">,

                       public nodes::ReactiveOut<xm::ocl::Image2D>,
                       public nodes::ReactiveIn<bool, int, const char*> {
    private:
        static inline const auto log = logs::get_logger("camera_node");

        camera::config config_;
        camera::Camera camera_;

    public:
        CameraNode() = default;

        ~CameraNode() override;

        camera::Camera *get() override;

        bool on_input(bool data, int input_index, int output_index, int clock_i) override;

        xm::ocl::Image2D on_output(xm::ocl::Image2D *null_ptr, int output_index, int clock_i) override;

        bool on_input(int data, int input_index, int output_index, int clock_i) override;

        bool on_input(const char *data, int input_index, int output_index, int clock_i) override;
    };

}

#endif //C_CAMERA_H
