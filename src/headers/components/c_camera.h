//
// Created by xd on 15/12/24.
//

#ifndef C_CAMERA_H
#define C_CAMERA_H

#include "../core/camera.h"
#include "../nodes/nodes.h"

namespace ex::cmp {

    #define CAM_       0
    #define CAM_FAST   1
    #define CAM_FLIP_H 2
    #define CAM_FLIP_V 3
    #define CAM_ROTATE 4
    #define CAM_WIDTH  5
    #define CAM_HEIGHT 6
    #define CAM_BUFFER 7
    #define CAM_FPS    8
    #define CAM_X      9
    #define CAM_Y      10
    #define CAM_W      11
    #define CAM_H      12
    #define CAM_CODEC  13
    #define CAM_UUID   14
    #define CAM_FREEZE 15
    #define CAM_ENABLE 16

    class CameraNode : public nodes::Node<CameraNode>,
                       public nodes::State<camera::Camera>,

                       public nodes::id::name<"camera_node">,

                       public nodes::io::out<xm::ocl::Image2D, 0, "Captured frame">,

                       public nodes::io::in<bool,         CAM_FAST,   "Fast mode">,
                       public nodes::io::in<bool,         CAM_FLIP_H, "Flip horizontal">,
                       public nodes::io::in<bool,         CAM_FLIP_V, "Flip vertical">,
                       public nodes::io::in<bool,         CAM_ROTATE, "Rotate">,
                       public nodes::io::in<bool,         CAM_FREEZE, "Freeze settings">,
                       public nodes::io::in<bool,         CAM_ENABLE, "ON/OFF camera">,
                       public nodes::io::in<int,          CAM_WIDTH,  "Width">,
                       public nodes::io::in<int,          CAM_HEIGHT, "Height">,
                       public nodes::io::in<int,          CAM_BUFFER, "Buffer">,
                       public nodes::io::in<int,          CAM_FPS,    "FPS">,
                       public nodes::io::in<int,          CAM_X,      "Region x">,
                       public nodes::io::in<int,          CAM_Y,      "Region y">,
                       public nodes::io::in<int,          CAM_W,      "Region width">,
                       public nodes::io::in<int,          CAM_H,      "Region height">,
                       public nodes::io::in<const char *, CAM_CODEC,  "CODEC">,
                       public nodes::io::in<const char *, CAM_UUID,   "UUID">,

                       public nodes::ReactiveOut<xm::ocl::Image2D>,
                       public nodes::ReactiveIn<bool, int, const char *> {
    private:
        static inline const auto log = logs::get_logger("camera_node");

        camera::config config_;
        camera::Camera camera_;

        bool initialized = false;
        bool enabled_    = false;
        bool updated_    = false;
        bool freeze_     = false;

    public:
        CameraNode() = default;

        ~CameraNode() override;

        camera::Camera *get() override;

        bool on_input(bool data, int input_index, int output_index, int clock_i) override;

        xm::ocl::Image2D on_output(xm::ocl::Image2D *null_ptr, int output_index, int clock_i) override;

        bool on_input(int data, int input_index, int output_index, int clock_i) override;

        bool on_input(const char *data, int input_index, int output_index, int clock_i) override;

    protected:
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
