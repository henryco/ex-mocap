//
// Created by xd on 15/12/24.
//

#ifndef C_CAMERA_H
#define C_CAMERA_H

#include "../cl/ocl_interop.h"
#include "../core/camera.h"
#include "../nodes/nodes.h"

namespace ex::cmp {

#define CAM_          0
#define CAM_OUT_FRAME 1
#define CAM_OUT_SYNC  2
#define CAM_IN_ENABLE 3

    using promise_i = xm::ocl::iop::ClImagePromise;

    class CameraNode : public nodes::Node<CameraNode>,
                       public nodes::State<camera::Camera>,

                       public nodes::id::name<"camera_node">,

                       public nodes::io::out<promise_i, CAM_OUT_FRAME, "Captured frame">,
                       public nodes::io::out<bool,      CAM_OUT_SYNC,  "Sync grabbed">,
                       public nodes::io::in<bool,       CAM_IN_ENABLE, "ON/OFF camera">,

                       public nodes::ReactiveOut<bool, promise_i>,
                       public nodes::ReactiveIn<bool>
    {
    private:
        static inline const auto log = logs::get_logger("camera_node");

        camera::Camera camera_;
        promise_i      last_frame_;

        bool enabled_    = true;
        int  last_clock_ = -1;

    public:
        CameraNode(cl_context context, cl_device_id device);

        CameraNode(const camera::Camera &camera);

        ~CameraNode() override;

        camera::Camera *get() override;

        bool on_input(bool data, int input_index, int _, int clock_i) override;

        promise_i on_output(promise_i *_, int __, int clock_i) override;

        bool on_output(bool *_, int __, int clock_i) override;

    protected:
        void capture_frame(int clock_i);
    };

}

#endif //C_CAMERA_H