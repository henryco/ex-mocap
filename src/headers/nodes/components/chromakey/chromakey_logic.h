//
// Created by xd on 01/06/25.
//

#ifndef EX_MOTION_CHROMAKEY_LOGIC_01062025_165013_H
#define EX_MOTION_CHROMAKEY_LOGIC_01062025_165013_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::chromakey {

    enum class mode_e : unsigned char {
        NONE   = 0U,
        ERODE  = 1U,
        DILATE = 2U,
        OPEN   = 3U, // erode  -> dilate
        CLOSE  = 4U, // dilate -> erode
    };

    struct hsl_t {
        float h; // normalized
        float s; // normalized
        float l; // normalized
    };

    struct bgra_t {
        float b; // normalized
        float g; // normalized
        float r; // normalized
        float a; // normalized
    };

    struct upscale_t {
        float scale;  // downscale factor
        bool  linear; // filtering
    };

    struct key_t {
        hsl_t key;
        float h_a;    // hue angle
        float s_t;    // saturation threshold
        float l_l;    // lightness low
        float l_h;    // lightness high
    };

    struct before_t {
        char  power;  // half kernel size (blur)
        bool  blur;   // blur enable/disable
    };

    struct refine_t {
        char   power; // kernel half size (morph)
        mode_e morph; // morph mode
    };

    struct config_t {
        upscale_t upscale;
        before_t  before;
        key_t     key;
        refine_t  refine;
        bgra_t    replace;
    };

    struct ChromakeyNode : limbo::graph::BaseNode,
                       limbo::graph::id::name<"chromakey_node">,
                       limbo::graph::io::in<ocl::img_promise, 1, "Frame input">,
                       limbo::graph::io::out<ocl::img_promise, 1, "Frame output">,
                       ex::io::bytes::serializable
    {
        // ---- exposed ----

        //  scale ─┐
        //  └─> before ─┐
        //      └─> mask  ─┐
        //          └─> refine ─┐
        //              └─> apply
        //
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
