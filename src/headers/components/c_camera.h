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

    class CameraNode : public nodes::Node<CameraNode>,
                       public nodes::State<camera::Camera>,

                       public nodes::id::name<"camera_node">,

                       public nodes::io::out<xm::ocl::iop::ClImagePromise, CAM_OUT_FRAME, "Captured frame">,
                       public nodes::io::out<bool,                         CAM_OUT_SYNC,  "Sync grabbed">,
                       public nodes::io::in<bool,                          CAM_IN_ENABLE, "ON/OFF camera">,

                       public nodes::ReactiveOut<bool, xm::ocl::iop::ClImagePromise>,
                       public nodes::ReactiveIn<bool> {
    private:
        static inline const auto log = logs::get_logger("camera_node");

        std::string    device_uuid;
        camera::config config_;
        camera::Camera camera_;

        bool                         enabled_    = true;
        int                          last_clock_ = -1;
        xm::ocl::iop::ClImagePromise last_frame_;

    public:
        CameraNode() = default;

        ~CameraNode() override;

        camera::Camera *get() override;

        bool on_input(bool data, int input_index, int _, int clock_i) override;

        xm::ocl::iop::ClImagePromise on_output(xm::ocl::iop::ClImagePromise *_, int __, int clock_i) override;

        bool on_output(bool *_, int __, int clock_i) override;

    protected:
        void capture_frame(int clock_i);
    };

}

#endif //C_CAMERA_H