#include <stdlib.h>
#include <string.h>
#include "event_timer.h"
#include "logger.h"  // Project-standard C logging interface

/* External reference to global event base (consistent with existing project APIs) */
extern struct event_base* event_loop_get_base(void);

/* Internal context for one-shot timers: stores user callback and auto-cleanup metadata */
typedef struct {
    timer_callback user_cb;  /**< User-registered callback function */
    void *user_arg;          /**< User-provided context for the callback */
    struct event *timer;     /**< Reference to the timer event for self-destruction */
} one_shot_ctx;

/* Internal context for periodic timers: stores user callback metadata */
typedef struct {
    timer_callback user_cb;  /**< User-registered callback function */
    void *user_arg;          /**< User-provided context for the callback */
} periodic_ctx;

/**
 * @brief Internal callback for one-shot timers
 * @note Automatically frees all allocated resources after invoking the user callback
 */
static void one_shot_internal_cb(evutil_socket_t fd, short what, void *arg) {
    one_shot_ctx *ctx = (one_shot_ctx *)arg;
    
    if (!(what & EV_TIMEOUT)) {
        log_warn("One-shot timer triggered with unexpected event mask: %d", what);
        return;
    }

    log_info("One-shot timer fired, executing user callback");
    if (ctx->user_cb) {
        ctx->user_cb(fd, what, ctx->user_arg);  // Invoke user-defined logic
    }

    /* Cleanup: one-shot timers are destroyed immediately after triggering */
    event_free(ctx->timer);
    free(ctx);
    log_debug("One-shot timer and associated context freed");
}

/**
 * @brief Internal callback for periodic timers
 * @note Does not free resources automatically - requires manual cancellation
 */
static void periodic_internal_cb(evutil_socket_t fd, short what, void *arg) {
    periodic_ctx *ctx = (periodic_ctx *)arg;
    
    if (!(what & EV_TIMEOUT)) {
        log_warn("Periodic timer triggered with unexpected event mask: %d", what);
        return;
    }

    log_info("Periodic timer fired, executing user callback");
    if (ctx->user_cb) {
        ctx->user_cb(fd, what, ctx->user_arg);  // Invoke user-defined logic
    }
}

struct event* register_one_shot_timer(unsigned int sec, unsigned int usec,
                                     timer_callback cb, void *arg) {
    struct event_base *base = event_loop_get_base();
    if (!base) {
        log_error("Event base not initialized - call event_loop_init() first");
        return NULL;
    }
    if (!cb) {
        log_error("One-shot timer requires a non-NULL callback function");
        return NULL;
    }

    /* Allocate internal context to track user data and timer reference */
    one_shot_ctx *ctx = malloc(sizeof(one_shot_ctx));
    if (!ctx) {
        log_error("Failed to allocate memory for one-shot timer context");
        return NULL;
    }
    memset(ctx, 0, sizeof(one_shot_ctx));
    ctx->user_cb = cb;
    ctx->user_arg = arg;

    /* Create timer event (EV_TIMEOUT marks this as a pure timer) */
    struct event *timer = evtimer_new(base, one_shot_internal_cb, ctx);
    if (!timer) {
        log_error("Failed to create one-shot timer event via evtimer_new()");
        free(ctx);
        return NULL;
    }
    ctx->timer = timer;

    /* Configure timeout duration */
    struct timeval tv = {.tv_sec = sec, .tv_usec = usec};

    /* Add timer to the event loop's schedule */
    if (event_add(timer, &tv) != 0) {
        log_error("Failed to schedule one-shot timer in event loop");
        event_free(timer);
        free(ctx);
        return NULL;
    }

    log_info("Successfully registered one-shot timer (timeout=%u.%06u seconds)", sec, usec);
    return timer;
}

struct event* register_periodic_timer(unsigned int sec, unsigned int usec,
                                     timer_callback cb, void *arg) {
    struct event_base *base = event_loop_get_base();
    if (!base) {
        log_error("Event base not initialized - call event_loop_init() first");
        return NULL;
    }
    if (!cb) {
        log_error("Periodic timer requires a non-NULL callback function");
        return NULL;
    }

    /* Allocate internal context to track user data */
    periodic_ctx *ctx = malloc(sizeof(periodic_ctx));
    if (!ctx) {
        log_error("Failed to allocate memory for periodic timer context");
        return NULL;
    }
    memset(ctx, 0, sizeof(periodic_ctx));
    ctx->user_cb = cb;
    ctx->user_arg = arg;

    /* Create periodic event (EV_PERSIST enables repeated triggering) */
    struct event *timer = event_new(base, -1, EV_PERSIST | EV_TIMEOUT,
                                   periodic_internal_cb, ctx);
    if (!timer) {
        log_error("Failed to create periodic timer event via event_new()");
        free(ctx);
        return NULL;
    }

    /* Configure repeat interval */
    struct timeval tv = {.tv_sec = sec, .tv_usec = usec};

    /* Add timer to the event loop's schedule */
    if (event_add(timer, &tv) != 0) {
        log_error("Failed to schedule periodic timer in event loop");
        event_free(timer);
        free(ctx);
        return NULL;
    }

    log_info("Successfully registered periodic timer (interval=%u.%06u seconds)", sec, usec);
    return timer;
}

static void periodic_timer_finalize(struct event *ev, void *arg) {
    periodic_ctx *ctx = (periodic_ctx *)arg;
    if (ctx) {
        free(ctx);
        log_debug("Periodic timer context freed safely via finalize");
    }
}

void unregister_timer(struct event *timer) {
    if (!timer) {
        log_warn("Attempted to unregister a NULL timer pointer");
        return;
    }
    
    /* Remove timer from event loop schedule and free allocated resources */
    //event_del(timer);
    //event_free(timer);

    event_free_finalize(EV_FINALIZE, timer, periodic_timer_finalize);
    log_debug("Timer successfully unregistered and destroyed");
}