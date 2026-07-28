#ifndef REACTOR_EVENT_LOOP_H
#define REACTOR_EVENT_LOOP_H

#include <event2/event.h>

/*
 * reactor/event_loop - Singleton libevent event_base wrapper
 *
 * Responsibilities:
 *   - Create and manage the ONE event_base for the entire program.
 *   - Provide run / break / cleanup primitives.
 *
 * Non-responsibilities:
 *   - Does NOT handle signals, timers, or IO. Those are owned by
 *     their respective modules under reactor/.
 *
 * Return convention:
 *   0 = success
 *   1 = failure
 */

/* Initialize the singleton event_base.
 * Return 0 on success, 1 on failure.
 */
int event_loop_init(void);

/* Get the singleton event_base pointer.
 * Must be called after event_loop_init().
 * Returns NULL if not initialized.
 */
struct event_base *event_loop_get_base(void);

/* Enter the event loop and block the calling thread.
 * Internally calls event_base_dispatch().
 * Return 0 on normal exit, 1 if not initialized.
 */
int event_loop_run(void);

/* Request the event loop to exit.
 * Safe to call from callbacks on the same thread.
 * Return 0 on success, 1 if base is not available.
 */
int event_loop_break(void);

/* Clean up: free the event_base.
 * After this call, event_loop_get_base() returns NULL.
 * Return 0 on success, 1 if already cleaned up.
 */
int event_loop_cleanup(void);

#endif /* REACTOR_EVENT_LOOP_H */
