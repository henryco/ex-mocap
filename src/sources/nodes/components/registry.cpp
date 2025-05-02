//
// Created by xd on 4/28/25.
//

#include "../../../headers/nodes/node_registry.h"

// TODO REMOVE
#include "../../../headers/nodes/components/test_cmp/test_cmp.h"
// TODO REMOVE

#include "../../../headers/nodes/components/image/image_gui.h"
#include "../../../headers/nodes/components/image/image_logic.h"

#include "../../../headers/nodes/components/camera/camera_gui.h"
#include "../../../headers/nodes/components/camera/camera_logic.h"

#include "../../../headers/nodes/components/sync/sync_gui.h"
#include "../../../headers/nodes/components/sync/sync_logic.h"

namespace ex::nodes::mappers {

    const data::array_map<id_type, Mapper> &cmp_registry_() {
        static data::array_map<id_type, Mapper> registry;

        { // TODO REMOVE
            registry.put(ex::gui::test::NodeOne::get_node_id(), {
                .gui = ex::gui::test::one::gui_mappers,
                .logic = ex::gui::test::one::data_mappers
            });
            registry.put(ex::gui::test::NodeTwo::get_node_id(), {
                .gui = ex::gui::test::two::gui_mappers,
                .logic = ex::gui::test::two::data_mappers
            });
            registry.put(ex::gui::test::NodeTri::get_node_id(), {
                .gui = ex::gui::test::tri::gui_mappers,
                .logic = ex::gui::test::tri::data_mappers
            });
        } // TODO REMOVE

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

        return registry;
    }

    const data::array_map<id_type, Mapper> &node_components() {
        static const data::array_map<id_type, Mapper> &instance = cmp_registry_();
        return instance;
    }
}