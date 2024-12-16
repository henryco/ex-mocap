//
// Created by xd on 15/12/24.
//

#ifndef C_CAMERA_H
#define C_CAMERA_H

#include "../core/camera.h"
#include "../nodes/nodes.h"

namespace ex::cmp {

    #define CAM_       0
    #define CAM_WIDTH  1
    #define CAM_HEIGHT 2
    #define CAM_BUFFER 3
    #define CAM_FPS    4
    #define CAM_CODEC  5
    #define CAM_UUID   6
    #define CAM_FREEZE 7
    #define CAM_ENABLE 8

    class CameraNode : public nodes::Node<CameraNode>,
                       public nodes::State<camera::Camera>,

                       public nodes::id::name<"camera_node">,

                       public nodes::io::out<xm::ocl::Image2D, 0, "Captured frame">,

                       public nodes::io::in<bool,         CAM_FREEZE, "Freeze settings">,
                       public nodes::io::in<bool,         CAM_ENABLE, "ON/OFF camera">,
                       public nodes::io::in<int,          CAM_WIDTH,  "Width">,
                       public nodes::io::in<int,          CAM_HEIGHT, "Height">,
                       public nodes::io::in<int,          CAM_BUFFER, "Buffer">,
                       public nodes::io::in<int,          CAM_FPS,    "FPS">,
                       public nodes::io::in<const char *, CAM_CODEC,  "CODEC">,
                       public nodes::io::in<const char *, CAM_UUID,   "UUID">,

                       public nodes::ReactiveOut<xm::ocl::Image2D>,
                       public nodes::ReactiveIn<bool, int, const char *> {
    private:
        static inline const auto log = logs::get_logger("camera_node");

        std::string    device_uuid;
        camera::config config_;
        camera::Camera camera_;

        bool initialized = false;
        bool enabled_    = false;
        bool updated_    = false;
        bool freeze_     = false;

        xm::ocl::Image2D last_frame_;
        int              last_clock_ = -1;

    public:
        CameraNode() = default;

        ~CameraNode() override;

        camera::Camera *get() override;

        bool on_input(bool data, int input_index, int output_index, int clock_i) override;

        xm::ocl::Image2D on_output(xm::ocl::Image2D *null_ptr, int output_index, int clock_i) override;

        bool on_input(int data, int input_index, int output_index, int clock_i) override;

        bool on_input(const char *data, int input_index, int output_index, int clock_i) override;

    protected:
        void grab_frame(int clock_i);

        template <typename T>
        bool update_value(T* ptr, T value) {
            if (*ptr != value)
                updated_ = true;
            *ptr = value;
            return true;
        }
    };

}

#endif //C_CAMERA_H
