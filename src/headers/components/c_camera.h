//
// Created by xd on 15/12/24.
//

#ifndef C_CAMERA_H
#define C_CAMERA_H

#include "../cl/ocl_interop.h"
#include "../core/camera.h"
#include "../nodes/nodes.h"
#include "image_protocol.h"

namespace ex::cmp {

#define CAM_          0
#define CAM_OUT_FRAME 1

    class CameraNode : public nodes::Node<CameraNode>,
                       public nodes::State<camera::Camera>,

                       public nodes::id::name<"camera_node">,

                       public nodes::io::out<proto::image, CAM_OUT_FRAME, "Captured frame">,

                       public nodes::ReactiveOut<proto::image>,
                       public nodes::NoInputs
    {
    private:
        static inline const auto log = logs::get_logger("camera_node");

        camera::Camera camera_;
        proto::image   last_frame_;

        int  last_clock_ = -1;

    public:
        CameraNode(cl_context context, cl_device_id device);

        CameraNode(const camera::Camera &camera);

        ~CameraNode() override;

        camera::Camera *get() override;

        proto::image on_output(proto::image *_, int __, int clock_i, nodes::Input<proto::image> *io) override;

    protected:
        void capture_frame(int clock_i);

        static bool sync_grab(void *ctx);
    };

}

#endif //C_CAMERA_H