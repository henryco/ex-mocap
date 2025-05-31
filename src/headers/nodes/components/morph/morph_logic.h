//
// Created by xd on 31/05/25.
//

#ifndef EX_MOTION_MORPH_LOGIC_31052025_170439_H
#define EX_MOTION_MORPH_LOGIC_31052025_170439_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::morph {

    enum class mode_e : unsigned char {
        ERODE  = 0U,
        DILATE = 1U,
        OPEN   = 2U, // erode  -> dilate
        CLOSE  = 3U, // dilate -> erode
    };

    struct config_t {
        mode_e mode;
        struct {
            char dilate;
            char erode;
        } kernel_size;
    };

    struct MorphNode : limbo::graph::BaseNode,
                       limbo::graph::id::name<"morph_node">,
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

        ~MorphNode() override;

        MorphNode();
    };
}

#endif // EX_MOTION_MORPH_LOGIC_31052025_170439_H
