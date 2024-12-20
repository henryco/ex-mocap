//
// Created by xd on 20/12/24.
//

#ifndef IMAGE_PROTOCOL_H
#define IMAGE_PROTOCOL_H

#include "../cl/ocl_interop.h"

namespace ex::proto {
    using promise_i = xm::ocl::iop::ClImagePromise;

    struct image {
        promise_i image_promise;
        bool (*sync)() = nullptr;
    };

    inline image from(const image &proto, const promise_i &promise) {
        return {promise, proto.sync};
    }

}

#endif //IMAGE_PROTOCOL_H
