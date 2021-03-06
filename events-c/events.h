/* 
 * Flexible event queue for dispatching events
 */
#ifndef EVENTS_H
#define EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

// System specific files
#include "events_tick.h"
#include "events_mutex.h"
#include "events_sema.h"


// Event/queue structures
struct event {
    struct event *next;
    int id;
    unsigned target;
    int period;
    void (*dtor)(void *);

    void (*cb)(void *);
    // data follows
};

typedef struct equeue {
    struct event *queue;
    int next_id;

    void *buffer;
    struct equeue_chunk {
        unsigned size;
        struct equeue_chunk *next;
        struct equeue_chunk *nchunk;
    } *chunks;
    struct equeue_slab {
        unsigned size;
        unsigned char *data;
    } slab;

    struct event break_;

    events_sema_t eventsema;
    events_mutex_t queuelock;
    events_mutex_t freelock;
} equeue_t;

// Queue operations
//
// Creation results in negative value on failure.
int equeue_create(equeue_t *queue, unsigned size);
int equeue_create_inplace(equeue_t *queue, unsigned size, void *buffer);
void equeue_destroy(equeue_t *queue);

// Dispatch events
//
// Executes any callbacks enqueued for the specified time in milliseconds,
// or forever if ms is negative
void equeue_dispatch(equeue_t *queue, int ms);

// Break a running event loop
//
// Shuts down an unbounded event loop. Already pending events may finish
// executing, but the queue will not continue looping indefinitely.
void equeue_break(equeue_t *queue);

// Simple event calls
//
// Passed callback will be executed in the associated equeue's
// dispatch call with the data pointer passed unmodified
//
// event_call       - Immediately post an event to the queue
// event_call_in    - Post an event after a specified time in milliseconds
// event_call_every - Post an event periodically in milliseconds
//
// These calls will result in 0 if no memory is available, otherwise they
// will result in a unique identifier that can be passed to event_cancel.
int event_call(equeue_t *queue, void (*cb)(void *), void *data);
int event_call_in(equeue_t *queue, int ms, void (*cb)(void *), void *data);
int event_call_every(equeue_t *queue, int ms, void (*cb)(void *), void *data);

// Events with queue handled blocks of memory
//
// Argument to event_post must point to a result of a event_alloc call
// and the associated memory is automatically freed after the event
// is dispatched.
//
// event_alloc will result in null if no memory is available
// or the requested size is less than the size passed to equeue_create.
void *event_alloc(equeue_t *queue, unsigned size);
void event_dealloc(equeue_t *queue, void *event);

// Configure an allocated event
// 
// event_delay  - Specify a millisecond delay before posting an event
// event_period - Specify a millisecond period to repeatedly post an event
// event_dtor   - Specify a destructor to run before the memory is deallocated
void event_delay(void *event, int ms);
void event_period(void *event, int ms);
void event_dtor(void *event, void (*dtor)(void *));

// Post an allocted event to the event queue
//
// Argument to event_post must point to a result of a event_alloc call
// and the associated memory is automatically freed after the event
// is dispatched.
//
// This call results in an unique identifier that can be passed to
// event_cancel.
int event_post(equeue_t *queue, void (*cb)(void *), void *event);

// Cancel events that are in flight
//
// Every event_call function returns a non-negative identifier on success
// that can be used to cancel an in-flight event. If the event has already
// been dispatched or does not exist, no error occurs. Note, this can not
// stop a currently executing event
void event_cancel(equeue_t *queue, int event);


#ifdef __cplusplus
}
#endif

#endif
