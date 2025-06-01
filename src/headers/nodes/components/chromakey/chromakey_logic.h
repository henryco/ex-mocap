//
// Created by xd on 01/06/25.
//

#ifndef EX_MOTION_CHROMAKEY_LOGIC_01062025_165013_H
#define EX_MOTION_CHROMAKEY_LOGIC_01062025_165013_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::chromakey {

    struct hsl_t {
        unsigned char h;
        unsigned char s;
        unsigned char l;
    };

    struct bgra_t {
        unsigned char b;
        unsigned char g;
        unsigned char r;
        unsigned char a;
    };

    struct key_t {
        hsl_t lo;
        hsl_t hi;
    };

    struct before_t {
        float scale;
        char  power;
        bool  blur;
    };

    struct after_t {
        char power;
        bool morph;
    };

    struct config_t {
        before_t before;
        key_t    key;
        bgra_t   replace;
        after_t  after;
    };

    struct ChromakeyNode : limbo::graph::BaseNode,
                       limbo::graph::id::name<"chromakey_node">,
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

        ~ChromakeyNode() override;

        ChromakeyNode();
    };

}

#endif // EX_MOTION_CHROMAKEY_LOGIC_01062025_165013_H
