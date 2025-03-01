//
// Created by henryco on 02/03/25.
//

#ifndef EX_SERIALIZER_H
#define EX_SERIALIZER_H

namespace ex::io {

    template <typename T>
    struct serializer {
        virtual ~serializer() = default;

        virtual T serialize(const void *) const = 0;

        virtual void *deserialize(const T &) const = 0;
    };

}

#endif //EX_SERIALIZER_H
