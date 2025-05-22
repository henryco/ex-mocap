//
// Created by xd on 22/05/25.
//

#ifndef EX_LIMBO_DATA_TIME_STACK_H
#define EX_LIMBO_DATA_TIME_STACK_H

namespace ex::data {

    template<typename key_t, typename val_t>
    using fn_t = void(key_t, const val_t &);

    template<unsigned sz, typename key_t, typename val_t, fn_t<key_t, val_t> fn>
    struct time_stack {

        val_t    data[sz];
        key_t    keys[sz];
        double   time[sz];
        unsigned size;

        void swap_remove(const int index) {
            if (size == 0)
                return;
            const val_t  tmp_val = data[index];
            const key_t  tmp_key = keys[index];
            const double tmp_tim = time[index];

            data[index]    = data[size - 1];
            keys[index]    = keys[size - 1];
            time[index]    = time[size - 1];
            data[size - 1] = tmp_val;
            keys[size - 1] = tmp_key;
            time[size - 1] = tmp_tim;
            size           = (size > 0) ? (size - 1) : 0;
        }

        void push(const double dt, const key_t key, const val_t &val) {
            for (int i = 0; i < size; ++i) {
                if (keys[i] == key) {
                    data[i] = val;
                    time[i] = dt;
                    return;
                }
            }

            if (size > 0)
                fn(keys[0], data[0]);
            swap_remove(0);

            data[size] = val;
            keys[size] = key;
            time[size] = dt;
            size++;
        }

        val_t *retrieve_consume(const key_t key, const double t_now, const double t_threshold) {
            for (int i = 0; i < size; ++i) {
                if (keys[i] == key) {
                    if (t_now - time[i] < t_threshold)
                        return &data[i]; // not yet
                    fn(key, data[i]);
                    swap_remove(i);
                    return nullptr; // should re-invalidate data
                }
            }
            return nullptr; // not in queue
        }

    };

}

#endif // EX_LIMBO_DATA_TIME_STACK_H
