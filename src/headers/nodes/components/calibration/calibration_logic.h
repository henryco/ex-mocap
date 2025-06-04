//
// Created by xd on 04/06/25.
//

#ifndef EX_MOTION_CALIBRATION_LOGIC_04062025_163105_H
#define EX_MOTION_CALIBRATION_LOGIC_04062025_163105_H

#include "../../../cl/cl_image.h"
#include "../../../cv/calibration.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::calibration {

    struct config_t {
        // Todo
    };

    struct CalibNode final : limbo::graph::BaseNode,
                             limbo::graph::id::name<"calibration_node">,
                             limbo::graph::io::in<ocl::img_promise, 1, "Frame input">,
                             limbo::graph::io::out<cv::socalib_t, 1, "Calibration data">,
                             ex::io::bytes::serializable
    {
        // ---- exposed ----

        // read only, writes changes nothing
        config_t config;


        // ---- internal ----

        struct internal_t *internal;


        // ---- mapper properties ----

        static limbo::graph::BaseNode *instance(TreeNode *);
        static limbo::graph::BaseNode *relocate(TreeNode *, limbo::graph::BaseNode *);
        static void                    destroy(TreeNode *, limbo::graph::BaseNode *);


        // ---- component properties ----

        void set_config(const config_t &);

        id_type_16 get_input_id() const;

        unsigned get_input_w() const;

        unsigned get_input_h() const;

        void disable();

        void enable();


        // ---- node properties ----

        io::byte_array serialize() const override;

        void deserialize(const io::byte_array &) override;

        void on_input(int in_idx, int out_idx, int clock_i, void *data) override;

        void on_update(int clock_i) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        ~CalibNode() override;

        CalibNode();
    };
}

#endif // EX_MOTION_CALIBRATION_LOGIC_04062025_163105_H
