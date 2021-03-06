/* events
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "events-c/events.h"
#include "Callback.h"
#include <cstddef>
#include <new>

namespace events {


/** DEFAULT_QUEUE_SIZE
 *  default size of buffer for events
 */
#define DEFAULT_QUEUE_SIZE \
    (32*(sizeof(struct event) + sizeof(mbed::Callback<void()>)))


/** EventQueue
 *
 *  Flexible event queue
 */
class EventQueue {
public:
    /** Create an event queue
     *
     *  @param queue_size       Size of buffer to use for events
     *                          (default: DEFAULT_QUEUE_SIZE)
     *  @param queue_pointer    Pointer to memory region to use for events
     *                          (default: NULL)
     */
    EventQueue(unsigned queue_size=DEFAULT_QUEUE_SIZE,
               unsigned char *queue_pointer=NULL);

    /** Destroy an event queue
     */
    ~EventQueue();

    /** Dispatch pending events
     *  @param ms   Time to wait for events in milliseconds, 0 will return
     *              immediately if no events are pending, a negative
     *              value will dispatch events forever
     *              (default: -1)
     */
    void dispatch(int ms=-1);

    /*  Monotonic counter for the event queue
     *  @return     A monotonically incrementing counter in milliseconds
     *              this count intentionally overflows to 0 after 2^32-1
     */
    unsigned get_tick();

    /** Cancel events that are in flight
     *
     *  If event has already been dispatched or does not exist, no error occurs.
     *
     *  @param id   Event id to cancel
     *  @note This can not stop a currently executing event
     */
    void cancel(int id);

    /** Post an event to the queue
     *
     *  @param f        Function to call on event dispatch
     *  @param a0..a4   Arguments to pass to the callback
     *  @return         A positive id representing the event in the queue,
     *                  or 0 on failure
     */
    template <typename F>
    int post(F f) {
        void *p = event_alloc(&_equeue, sizeof(F));
        if (!p) {
            return 0;
        }

        F *e = new (p) F(f);
        event_dtor(e, &EventQueue::dtor<F>);
        return event_post(&_equeue, &EventQueue::call<F>, e);
    }

    template <typename F, typename A0>
    int post(F f, A0 a0) {
        return post(Context1<F,A0>(f,a0));
    }

    template <typename F, typename A0, typename A1>
    int post(F f, A0 a0, A1 a1) {
        return post(Context2<F,A0,A1>(f,a0,a1));
    }

    template <typename F, typename A0, typename A1, typename A2>
    int post(F f, A0 a0, A1 a1, A2 a2) {
        return post(Context3<F,A0,A1,A2>(f,a0,a1,a2));
    }

    template <typename F, typename A0, typename A1, typename A2, typename A3>
    int post(F f, A0 a0, A1 a1, A2 a2, A3 a3) {
        return post(Context4<F,A0,A1,A2,A3>(f,a0,a1,a2,a3));
    }

    template <typename F, typename A0, typename A1, typename A2, typename A3, typename A4>
    int post(F f, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        return post(Context5<F,A0,A1,A2,A3,A4>(f,a0,a1,a2,a3,a4));
    }

    /** Post an event to the queue after a specified delay
     *
     *  @param f        Function to call on event dispatch
     *  @param a0..a4   Arguments to pass to the callback
     *  @param ms       Time to delay in milliseconds
     *  @return         A positive id representing the event in the queue,
     *                  or 0 on failure
     */
    template <typename F>
    int post_in(int ms, F f) {
        void *p = event_alloc(&_equeue, sizeof(F));
        if (!p) {
            return 0;
        }

        F *e = new (p) F(f);
        event_delay(e, ms);
        event_dtor(e, &EventQueue::dtor<F>);
        return event_post(&_equeue, &EventQueue::call<F>, e);
    }

    template <typename F, typename A0>
    int post_in(int ms, F f, A0 a0) {
        return post_in(ms, Context1<F,A0>(f,a0));
    }

    template <typename F, typename A0, typename A1>
    int post_in(int ms, F f, A0 a0, A1 a1) {
        return post_in(ms, Context2<F,A0,A1>(f,a0,a1));
    }

    template <typename F, typename A0, typename A1, typename A2>
    int post_in(int ms, F f, A0 a0, A1 a1, A2 a2) {
        return post_in(ms, Context3<F,A0,A1,A2>(f,a0,a1,a2));
    }

    template <typename F, typename A0, typename A1, typename A2, typename A3>
    int post_in(int ms, F f, A0 a0, A1 a1, A2 a2, A3 a3) {
        return post_in(ms, Context4<F,A0,A1,A2,A3>(f,a0,a1,a2,a3));
    }

    template <typename F, typename A0, typename A1, typename A2, typename A3, typename A4>
    int post_in(int ms, F f, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        return post_in(ms, Context5<F,A0,A1,A2,A3,A4>(f,a0,a1,a2,a3,a4));
    }

    /** Post an event to the queue periodically
     *
     *  @param f        Function to call on event dispatch
     *  @param a0..a4   Arguments to pass to the callback
     *  @param ms       Period of the event in milliseconds
     *  @return         A positive id representing the event in the queue,
     *                  or 0 on failure
     */
    template <typename F>
    int post_every(int ms, F f) {
        void *p = event_alloc(&_equeue, sizeof(F));
        if (!p) {
            return 0;
        }

        F *e = new (p) F(f);
        event_delay(e, ms);
        event_period(e, ms);
        event_dtor(e, &EventQueue::dtor<F>);
        return event_post(&_equeue, &EventQueue::call<F>, e);
    }

    template <typename F, typename A0>
    int post_every(int ms, F f, A0 a0) {
        return post_every(ms, Context1<F,A0>(f,a0));
    }

    template <typename F, typename A0, typename A1>
    int post_every(int ms, F f, A0 a0, A1 a1) {
        return post_every(ms, Context2<F,A0,A1>(f,a0,a1));
    }

    template <typename F, typename A0, typename A1, typename A2>
    int post_every(int ms, F f, A0 a0, A1 a1, A2 a2) {
        return post_every(ms, Context3<F,A0,A1,A2>(f,a0,a1,a2));
    }

    template <typename F, typename A0, typename A1, typename A2, typename A3>
    int post_every(int ms, F f, A0 a0, A1 a1, A2 a2, A3 a3) {
        return post_every(ms, Context4<F,A0,A1,A2,A3>(f,a0,a1,a2,a3));
    }

    template <typename F, typename A0, typename A1, typename A2, typename A3, typename A4>
    int post_every(int ms, F f, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        return post_every(ms, Context5<F,A0,A1,A2,A3,A4>(f,a0,a1,a2,a3,a4));
    }

protected:
    void break_();

    struct equeue _equeue;

    template <typename F, typename A0, typename A1, typename A2, typename A3, typename A4>
    struct Context5 {
        F f; A0 a0; A1 a1; A2 a2; A3 a3; A4 a4;

        Context5(F f, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
            : f(f), a0(a0), a1(a1), a2(a2), a3(a3), a4(a4) {}

        void operator()() {
            f(a0, a1, a2, a3, a4);
        }
    };

    template <typename F, typename A0, typename A1, typename A2, typename A3>
    struct Context4 {
        F f; A0 a0; A1 a1; A2 a2; A3 a3;

        Context4(F f, A0 a0, A1 a1, A2 a2, A3 a3)
            : f(f), a0(a0), a1(a1), a2(a2), a3(a3) {}

        void operator()() {
            f(a0, a1, a2, a3);
        }
    };

    template <typename F, typename A0, typename A1, typename A2>
    struct Context3 {
        F f; A0 a0; A1 a1; A2 a2;

        Context3(F f, A0 a0, A1 a1, A2 a2)
            : f(f), a0(a0), a1(a1), a2(a2) {}

        void operator()() {
            f(a0, a1, a2);
        }
    };

    template <typename F, typename A0, typename A1>
    struct Context2 {
        F f; A0 a0; A1 a1;

        Context2(F f, A0 a0, A1 a1)
            : f(f), a0(a0), a1(a1) {}

        void operator()() {
            f(a0, a1);
        }
    };

    template <typename F, typename A0>
    struct Context1 {
        F f; A0 a0;

        Context1(F f, A0 a0)
            : f(f), a0(a0) {}

        void operator()() {
            f(a0);
        }
    };

    template <typename T>
    static void call(void *p) {
        (*static_cast<T*>(p))();
    }

    template <typename T>
    static void dtor(void *p) {
        static_cast<T*>(p)->~T();
    }
};


}

#endif
