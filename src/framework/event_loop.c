#include "event_loop.h"
#include "logger.h"

#include <stdio.h>

/* ---------- Internal state (hidden from all callers) ---------- */

static struct event_base *g_base = NULL;

/* ---------- API implementation ---------- */

int event_loop_init(void) {
    if (g_base != NULL) {
        /* Already initialized; idempotent success */
        return 0;
    }

    g_base = event_base_new();
    if (g_base == NULL) {
        log_error("event_loop: failed to create event_base");
        return 1;
    }

    log_info("event_loop: initialized (base=%p)", (void *)g_base);
    return 0;
}

struct event_base *event_loop_get_base(void) {
    return g_base;
}

int event_loop_run(void) {
    if (g_base == NULL) {
        log_error("event_loop: run() called before init()");
        return 1;
    }

    log_info("event_loop: dispatching ...");
    event_base_dispatch(g_base);
    log_info("event_loop: dispatch returned, loop exited");

    return 0;
}

int event_loop_break(void) {
    if (g_base == NULL) {
        return 1;
    }

    event_base_loopbreak(g_base);
    return 0;
}

int event_loop_cleanup(void) {
    if (g_base == NULL) {
        /* Already cleaned up */
        return 1;
    }

    event_base_free(g_base);
    g_base = NULL;

    log_info("event_loop: cleaned up");
    return 0;
}
