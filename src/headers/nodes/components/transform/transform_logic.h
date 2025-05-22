//
// Created by xd on 18/05/25.
//

#ifndef EX_NODES_CMP_TRANSFORM_LOGIC_H
#define EX_NODES_CMP_TRANSFORM_LOGIC_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::transform {

    enum class rotate_e: unsigned char {
        NONE = 0U, //  0º
        CW_1 = 1U, // 90º
    };

    struct flip_t {
        bool flip_h;
        bool flip_v;
    };

    struct crop_t {
        unsigned start_x;
        unsigned start_y;
        unsigned end_x;
        unsigned end_y;
    };

    struct resize_t {
        unsigned width;
        unsigned height;
        bool     linear;
    };

    struct config_t {
        crop_t   crop;
        // │
        // V
        resize_t resize;
        // │
        // V
        flip_t   flip;
        // │
        // V
        rotate_e rotate;
    };

    struct TransformNode final : limbo::graph::BaseNode,
                                 limbo::graph::id::name<"transform_node">,
                                 limbo::graph::io::in<ocl::img_promise, 1, "Frame input">,
                                 limbo::graph::io::out<ocl::img_promise, 1, "Frame output">,
                                 ex::io::bytes::serializable
    {
        // ---- exposed ----

        //  crop ─┐
        //   └─> resize ─┐
        //        └─> flip ─┐
        //             └─> rotate
        // read only
        config_t config;


        // ---- internal ----

        struct internal_t *internal;


        // ---- mapper properties ----

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);


        // ---- component properties ----

        void set_image_id(id_type_16 id);

        id_type_16 get_image_id() const;

        void disable();

        void enable();

        void reset();

        unsigned get_image_w() const;

        unsigned get_image_h() const;

        void update(const config_t &);

        // ---- node properties ----

        io::byte_array serialize() const override;

        void deserialize(const io::byte_array &) override;

        void on_input(int in_idx, int out_idx, int clock_i, void *data) override;

        void on_update(int clock_i) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        ~TransformNode() override;

        TransformNode();
    };

}

#endif //EX_NODES_CMP_TRANSFORM_LOGIC_H