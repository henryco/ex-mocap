//
// Created by xd on 29/05/25.
//

#ifndef EX_NODES_CMP_BLUR_LOGIC_H
#define EX_NODES_CMP_BLUR_LOGIC_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::blur {

    enum class mode_e : unsigned char {
        AUTO   = 0U,
        SINGLE = 1U,
        DOUBLE = 2U
    };

    struct config_t {
        // standard deviation
        float sigma;

        // single is faster for small kernels
        mode_e mode;

        // hard-clamped to [0,20]
        char kernel_size;
    };

    struct BlurNode final : limbo::graph::BaseNode,
                            limbo::graph::id::name<"blur_node">,
                            limbo::graph::io::in<ocl::img_promise, 1, "Frame input">,
                            limbo::graph::io::out<ocl::img_promise, 1, "Frame output">,
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

        void set_image_id(id_type_16);

        id_type_16 get_image_id() const;

        id_type_16 get_input_id() const;

        unsigned get_image_w() const;

        unsigned get_image_h() const;

        void disable();

        void enable();


        // ---- node properties ----

        io::byte_array serialize() const override;

        void deserialize(const io::byte_array &) override;

        void on_input(int in_idx, int out_idx, int clock_i, void *data) override;

        void on_update(int clock_i) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        ~BlurNode() override;

        BlurNode();
    };

}

#endif // EX_NODES_CMP_BLUR_LOGIC_H
