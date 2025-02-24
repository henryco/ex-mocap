//
// Created by xd on 15/12/24.
//

#ifndef C_CAMERA_H
#define C_CAMERA_H

#include "../../cl/ocl_interop.h"
#include "../../core/camera.h"
#include "../../core/image_protocol.h"
#include "../logic_nodes.h"

namespace ex::map::camera {
    limbo::nodes::BaseNode            *instance(TreeNode *from);
    limbo::nodes::BaseNode            *relocate(TreeNode *tree, limbo::nodes::BaseNode *node);
    static constexpr logic::NodeMapper mappers = { instance, relocate };
}

namespace ex::cmp {

    #define CAM_          0
    #define CAM_OUT_FRAME 1

    class CameraNode : public limbo::nodes::Node<CameraNode>,
                       public limbo::nodes::State<camera::Camera>,

                       public limbo::nodes::id::name<"camera_node">,

                       public limbo::nodes::io::out<proto::image, CAM_OUT_FRAME, "Captured frame">,

                       public limbo::nodes::ReactiveOut<proto::image>,
                       public limbo::nodes::NoInputs
    {
    private:
        static inline const auto log = logs::get_logger("camera_node");

        camera::Camera camera_;
        proto::image   last_frame_;

        int  last_clock_ = -1;

    public:
        CameraNode(cl_context context, cl_device_id device);

        explicit CameraNode(const camera::Camera &camera);

        ~CameraNode() override;

        camera::Camera *get() override;

        proto::image on_output(proto::image *, int, int clock_i, limbo::nodes::Input<proto::image> *io) override;

    protected:
        void capture_frame(int clock_i);

        static bool sync_grab(void *ctx);
    };

}

#endif //C_CAMERA_H