#ifndef REACTOR_SIGNAL_HANDLER_H
#define REACTOR_SIGNAL_HANDLER_H

#include <event2/event.h>

/*
 * reactor/signal_handler - POSIX signal registration on libevent
 *
 * Registers SIGINT and SIGTERM on the given event_base.
 * On receipt, the event loop is gracefully stopped via loopbreak.
 *
 * Return convention:
 *   0 = success
 *   1 = failure
 */

/* Opaque handle for the signal handler */
struct signal_handler;

/* Create a signal_handler bound to the given event_base.
 * The base MUST outlive this signal_handler.
 * Return NULL on failure.
 */
struct signal_handler *signal_handler_new(struct event_base *base);

/* Register SIGINT and SIGTERM with the event loop.
 * Return 0 on success, 1 on failure.
 */
int signal_handler_start(struct signal_handler *h);

/* Free all resources held by the signal handler.
 * Safe to call with NULL.
 */
void signal_handler_free(struct signal_handler *h);

#endif /* REACTOR_SIGNAL_HANDLER_H */
