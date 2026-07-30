#include "event_loop.h"

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
        fprintf(stderr, "event_loop: failed to create event_base\n");
        return 1;
    }

    printf("event_loop: initialized (base=%p)\n", (void *)g_base);
    return 0;
}

struct event_base *event_loop_get_base(void) {
    return g_base;
}

int event_loop_run(void) {
    if (g_base == NULL) {
        fprintf(stderr, "event_loop: run() called before init()\n");
        return 1;
    }

    printf("event_loop: dispatching (Ctrl+C to stop)...\n");
    event_base_dispatch(g_base);
    printf("event_loop: dispatch returned, loop exited\n");

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

    printf("event_loop: cleaned up\n");
    return 0;
}
