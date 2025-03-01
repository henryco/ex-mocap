//
// Created by xd on 15/12/24.
//

#ifndef C_CAMERA_H
#define C_CAMERA_H

#include "../../../cl/ocl_interop.h"
#include "../../../core/camera.h"
#include "../../../core/image_protocol.h"
#include "../logic_nodes.h"

namespace ex::map::camera {
    limbo::graph::BaseNode            *instance(TreeNode *from);
    limbo::graph::BaseNode            *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
    static constexpr logic::NodeMapper mappers = { instance, relocate };
}

namespace ex::cmp {

    #define CAM_          0
    #define CAM_OUT_FRAME 1

    struct CameraNode final: limbo::graph::BaseNode,
                        limbo::graph::State<camera::Camera>,
                        limbo::graph::id::name<"camera_node">,
                        limbo::graph::io::out<proto::image, CAM_OUT_FRAME, "Captured frame">
    {
        static inline const auto log = logs::get_logger("camera_node");

        camera::Camera camera_;
        proto::image   last_frame_;

        int  last_clock_ = -1;

        CameraNode(cl_context context, cl_device_id device);

        explicit CameraNode(const camera::Camera &camera);

        ~CameraNode() override;

        camera::Camera *get() override;

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override;

        void on_output(int in_idx, int out_idx, int clock_i, void *data) override;

        void capture_frame(int clock_i);

        static bool sync_grab(void *ctx);
    };

}

#endif //C_CAMERA_H