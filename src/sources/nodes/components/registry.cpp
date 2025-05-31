//
// Created by xd on 4/28/25.
//

#include "../../../headers/nodes/node_registry.h"

#include "../../../headers/nodes/components/image/image_gui.h"
#include "../../../headers/nodes/components/image/image_logic.h"

#include "../../../headers/nodes/components/camera/camera_gui.h"
#include "../../../headers/nodes/components/camera/camera_logic.h"

#include "../../../headers/nodes/components/transform/transform_gui.h"
#include "../../../headers/nodes/components/transform/transform_logic.h"

#include "../../../headers/nodes/components/blur/blur_gui.h"
#include "../../../headers/nodes/components/blur/blur_logic.h"

#include "../../../headers/nodes/components/morph/morph_gui.h"
#include "../../../headers/nodes/components/morph/morph_logic.h"

namespace ex::nodes::mappers {

    const data::array_map<id_type, Mapper> &cmp_registry_() {
        static data::array_map<id_type, Mapper> registry;

        { // Camera component
            namespace camera_node_n = ex::gui::node::camera;
            using camera_node_t     = ex::logic::node::camera::CameraNode;
            registry.put(id_from_struct_<camera_node_t>(), {
                    .gui = camera_node_n::gui_mappers,
                    .logic = logic_from_struct_<camera_node_t>()
            });
        }

        { // Image component
            namespace image_node_n = ex::gui::node::image;
            using image_node_t     = ex::logic::node::image::ImageNode;
            registry.put(id_from_struct_<image_node_t>(), {
                    .gui = image_node_n::gui_mappers,
                    .logic = logic_from_struct_<image_node_t>()
            });
        }

        { // Sync component
            namespace sync_node_n = ex::gui::node::sync;
            using sync_node_t     = ex::logic::node::sync::SyncNode;
            registry.put(id_from_struct_<sync_node_t>(), {
                    .gui = sync_node_n::gui_mappers,
                    .logic = logic_from_struct_<sync_node_t>()
            });
        }

        { // Transform component
            namespace transform_node_n = ex::gui::node::transform;
            using transform_node_t     = ex::logic::node::transform::TransformNode;
            registry.put(id_from_struct_<transform_node_t>(), {
                    .gui = transform_node_n::gui_mappers,
                    .logic = logic_from_struct_<transform_node_t>()
            });
        }

        { // Blur component
            namespace blur_node_n = ex::gui::node::blur;
            using blur_node_t     = ex::logic::node::blur::BlurNode;
            registry.put(id_from_struct_<blur_node_t>(), {
                    .gui = blur_node_n::gui_mappers,
                    .logic = logic_from_struct_<blur_node_t>()
          });
        }

        { // Morph component
            namespace morph_node_n = ex::gui::node::morph;
            using morph_node_t     = ex::logic::node::morph::MorphNode;
            registry.put(id_from_struct_<morph_node_t>(), {
                    .gui = morph_node_n::gui_mappers,
                    .logic = logic_from_struct_<morph_node_t>()
            });
        }

        return registry;
    }

    const data::array_map<id_type, Mapper> &node_components() {
        static const data::array_map<id_type, Mapper> &instance = cmp_registry_();
        return instance;
    }
}