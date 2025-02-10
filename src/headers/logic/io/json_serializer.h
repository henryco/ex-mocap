//
// Created by xd on 01/02/25.
//

#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H
#include "../../../extra/nodes/nodes.h"

namespace ex::io::json {

    class JsonSerializer : public ex::nodes::Serializer<std::string> {
    public:
        JsonSerializer()           = default;
        ~JsonSerializer() override = default;

        std::string serialize(const void *) const override;

        void *deserialize(const std::string &) const override;
    };

}

#endif // JSON_SERIALIZER_H
