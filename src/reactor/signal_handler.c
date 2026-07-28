#include "signal_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <event2/event.h>

/* ---------- Internal types ---------- */

struct signal_handler {
    struct event_base *base;
    struct event     *sigint_ev;
    struct event     *sigterm_ev;
};

/* ---------- Signal callback ---------- */

static void signal_cb(evutil_socket_t sig, short events, void *arg) {
    struct event_base *base = (struct event_base *)arg;

    printf("signal_handler: received signal %d, stopping loop...\n", (int)sig);

    /* loopbreak: exit dispatch() immediately after this callback returns */
    event_base_loopbreak(base);
}

/* ---------- API implementation ---------- */

struct signal_handler *signal_handler_new(struct event_base *base) {
    if (base == NULL) {
        return NULL;
    }

    struct signal_handler *h = calloc(1, sizeof(*h));
    if (h == NULL) {
        fprintf(stderr, "signal_handler: calloc failed\n");
        return NULL;
    }

    h->base = base;
    return h;
}

int signal_handler_start(struct signal_handler *h) {
    if (h == NULL || h->base == NULL) {
        return 1;
    }

    h->sigint_ev  = evsignal_new(h->base, SIGINT,  signal_cb, h->base);
    h->sigterm_ev = evsignal_new(h->base, SIGTERM, signal_cb, h->base);

    if (h->sigint_ev == NULL || h->sigterm_ev == NULL) {
        fprintf(stderr, "signal_handler: failed to create signal events\n");
        return 1;
    }

    if (event_add(h->sigint_ev, NULL) != 0) {
        fprintf(stderr, "signal_handler: failed to add SIGINT\n");
        return 1;
    }

    if (event_add(h->sigterm_ev, NULL) != 0) {
        fprintf(stderr, "signal_handler: failed to add SIGTERM\n");
        return 1;
    }

    printf("signal_handler: SIGINT/SIGTERM registered\n");
    return 0;
}

void signal_handler_free(struct signal_handler *h) {
    if (h == NULL) {
        return;
    }

    if (h->sigint_ev != NULL) {
        event_free(h->sigint_ev);
        h->sigint_ev = NULL;
    }

    if (h->sigterm_ev != NULL) {
        event_free(h->sigterm_ev);
        h->sigterm_ev = NULL;
    }

    free(h);
    printf("signal_handler: freed\n");
}
