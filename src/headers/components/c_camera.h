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
                       public nodes::io::in <bool,             1, "Fast mode">,

                       public nodes::ReactiveOut<xm::ocl::Image2D>,
                       public nodes::ReactiveIn<bool> {
    private:
        static inline const auto log = logs::get_logger("camera_node");

        camera::Camera camera_;

    public:
        CameraNode() = default;

        ~CameraNode() override;

        const camera::Camera *get() const override;

        bool on_input(bool data, int input_index, int output_index, int clock_i) override;

        xm::ocl::Image2D on_output(xm::ocl::Image2D *null_ptr, int output_index, int clock_i) override;
    };

}

#endif //C_CAMERA_H
