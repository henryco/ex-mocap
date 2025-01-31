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
        bool (*sync_)(void *ctx) = nullptr;
        void *ctx___             = nullptr;

        bool sync() const {
            return sync_(ctx___);
        }
    };

    inline image from(const image &proto, const promise_i &promise) {
        return {promise, proto.sync_, proto.ctx___};
    }

}

#endif //IMAGE_PROTOCOL_H
