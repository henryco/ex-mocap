//
// Created by henryco on 13/06/24.
//

#ifndef XMOTION_OCL_INTEROP_EXT_H
#define XMOTION_OCL_INTEROP_EXT_H

#include <CL/cl.h>
#include <functional>
#include <memory>

#include "ocl_container.h"
#include "../../headers/utils/err_utils.h"

namespace xm::ocl::iop {

    template <typename T>
    class CLPromise {
    protected:
        std::shared_ptr<ResourceContainer> cleanup_container = nullptr;
        cl_command_queue ocl_queue = nullptr;
        cl_event ocl_event = nullptr;
        bool completed = false;
        T data;

    public:
        CLPromise(T obj, const cl_command_queue queue, const cl_event event = nullptr):
                ocl_queue(queue), ocl_event(event), completed(false), data(obj) {
            if (queue != nullptr)
                clRetainCommandQueue(queue);
            if (event != nullptr)
                clRetainEvent(event);
        }

        CLPromise(T obj, const cl_event event = nullptr): // NOLINT(*-explicit-constructor)
                ocl_queue(nullptr), ocl_event(event), completed(true), data(obj) {
            if (event != nullptr)
                clRetainEvent(event);
        }

        CLPromise() = default;

        ~CLPromise() {
            release();
        }

        CLPromise(CLPromise &&other) noexcept {
            cleanup_container = std::move(other.cleanup_container);
            completed = other.completed;
            ocl_queue = other.ocl_queue;
            ocl_event = other.ocl_event;
            data = std::move(other.data);
            other.ocl_event = nullptr;
            other.ocl_queue = nullptr;
            other.completed = true;
        }

        CLPromise(const CLPromise &other) {
            cleanup_container = other.cleanup_container;
            completed = other.completed;
            ocl_queue = other.ocl_queue;
            ocl_event = other.ocl_event;
            data = other.data;
            clRetainEvent(ocl_event);
            clRetainCommandQueue(ocl_queue);
        }

        CLPromise &operator=(CLPromise &&other) noexcept {
            if (this != &other) {
                if (ocl_event != nullptr)
                    clReleaseEvent(ocl_event);
                if (ocl_queue != nullptr)
                    clReleaseCommandQueue(ocl_queue);
                cleanup_container = std::move(other.cleanup_container);
                completed = other.completed;
                ocl_queue = other.ocl_queue;
                ocl_event = other.ocl_event;
                data = std::move(other.data);
                other.ocl_event = nullptr;
                other.ocl_queue = nullptr;
                other.completed = true;
            }
            return *this;
        }

        CLPromise &operator=(const CLPromise &other) {
            if (this == &other)
                *this;
            if (ocl_event != nullptr)
                clReleaseEvent(ocl_event);
            if (ocl_queue != nullptr)
                clReleaseCommandQueue(ocl_queue);
            cleanup_container = other.cleanup_container;
            completed = other.completed;
            ocl_queue = other.ocl_queue;
            ocl_event = other.ocl_event;
            data = other.data;
            clRetainEvent(ocl_event);
            clRetainCommandQueue(ocl_queue);
            return *this;
        }

        /**
         * Often you should call waitFor() first
         */
        T get() const {
            return data;
        }

        /**
         * Most of the time called by constructor
         */
        void release() {
            if (ocl_event != nullptr)
                clReleaseEvent(ocl_event);
            if (ocl_queue != nullptr)
                clReleaseCommandQueue(ocl_queue);
            ocl_event = nullptr;
            ocl_queue = nullptr;
        }

        /**
         * Cleanup resources (dependencies)
         * @param force cleanup even if not completed
         */
        void cleanup(const bool force = false) {
            if (!completed && !force)
                return;
            if (cleanup_container) {
                (*cleanup_container)();
                cleanup_container = nullptr;
            }
        }

        /**
         * Waits for data to be ready
         */
        CLPromise &waitFor() {
            if (completed)
                return *this;
            {
                cl_int err;

                if (ocl_queue) err = clFinish(ocl_queue);
                else err             = CL_SUCCESS;

                if (err != CL_SUCCESS)
                    abort__("Cannot finish command queue: %d", err);
                completed = true;

                if (cleanup_container) {
                    (*cleanup_container)();
                    cleanup_container = nullptr;
                }
            }
            return *this;
        }

        /**
         * BLOCKING OPERATION, waits for result and cleanup resources
         * @param promises list of unique promises
         * @param force wait for completed promises too
         */
        static void finalizeAll(std::vector<CLPromise> &promises, const bool force = false) {
            static thread_local cl_command_queue list[256];
            int n = 0;
            // this is narrow/naive map implementation, but for reasonably small input size
            // this would be much faster then proper nlog(n) map/set due to much smaller overhead,
            // which is exactly the case here
            for (auto &p: promises) {
                if (p.completed && !force)
                    continue;

                p.completed = true;

                bool found = false;
                for (int i = 0; i < n; i++) {
                    if (list[i] != p.ocl_queue) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    list[n++] = p.ocl_queue;
                    goto waiting_room;
                }

                if (p.cleanup_container) {
                    (*p.cleanup_container)();
                    p.cleanup_container = nullptr;
                }

                continue;

                waiting_room:
                {
                    cl_int err;

                    if (p.ocl_queue) err = clFinish(p.ocl_queue);
                    else err             = CL_SUCCESS;

                    if (err != CL_SUCCESS) {
                        abort__("Cannot finish command queue: %d", err);
                    }
                    if (p.cleanup_container) {
                        (*p.cleanup_container)();
                        p.cleanup_container = nullptr;
                    }
                }
            }
        }

        /**
         * BLOCKING OPERATION, waits for result and cleanup resources
         * @param promises array of unique promises
         * @param size size of the array
         * @param force wait for completed promises too
         */
        static void finalizeAll(CLPromise *promises, const size_t size, const bool force = false) {
            static thread_local cl_command_queue list[256];
            int n = 0;
            // this is narrow/naive map implementation, but for reasonably small input size
            // this would be much faster then proper nlog(n) map/set due to much smaller overhead,
            // which is exactly the case here
            for (int g = 0; g < size; g++) {
                auto &p = promises[g];

                if (p.completed && !force)
                    continue;

                p.completed = true;

                bool found = false;
                for (int i = 0; i < n; i++) {
                    if (list[i] != p.ocl_queue) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    list[n++] = p.ocl_queue;
                    goto waiting_room;
                }

                if (p.cleanup_container) {
                    (*p.cleanup_container)();
                    p.cleanup_container = nullptr;
                }

                continue;

                waiting_room:
                {
                    cl_int err;

                    if (p.ocl_queue) err = clFinish(p.ocl_queue);
                    else err             = CL_SUCCESS;

                    if (err != CL_SUCCESS) {
                        abort__("Cannot finish command queue: %d", err);
                    }
                    if (p.cleanup_container) {
                        (*p.cleanup_container)();
                        p.cleanup_container = nullptr;
                    }
                }
            }
        }

        CLPromise &withCleanup(std::function<void()> *cb_ptr) {
            if (!cb_ptr)
                return *this;

            if (!cleanup_container) {
                cleanup_container = std::make_shared<ResourceContainer>(cb_ptr);
                return *this;
            }

            auto current = cleanup_container;
            auto another = std::make_shared<ResourceContainer>(cb_ptr);
            cleanup_container = std::make_shared<ResourceContainer>(new std::function<void()>(
                    [current, another] {
                        (*current)();
                        (*another)();
                    }));

            return *this;
        }

        CLPromise &withCleanup(const CLPromise &other) {
            if (this == &other)
                return *this;

            if (!other.cleanup_container)
                return *this;

            if (!cleanup_container) {
                cleanup_container = other.cleanup_container;
                return *this;
            }

            auto current = cleanup_container;
            cleanup_container = std::make_shared<ResourceContainer>(new std::function<void()>(
                    [current, another = other.cleanup_container] {
                        (*current)();
                        (*another)();
                    }));

            return *this;
        }

        bool resolved() const {
            return completed;
        }

        const cl_event &event() const {
            return ocl_event;
        }

        cl_command_queue queue() const {
            return ocl_queue;
        }
    };

}

#endif //XMOTION_OCL_INTEROP_EXT_H
