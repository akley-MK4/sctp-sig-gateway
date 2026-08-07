#ifndef EVENT_TIMER_H
#define EVENT_TIMER_H

#include <event2/event.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Timer callback function signature, compliant with libevent standard callbacks
 * @param fd Unused for timers (always -1)
 * @param what Event flags (will always include EV_TIMEOUT for timer triggers)
 * @param arg User-provided context pointer passed during timer registration
 */
typedef void (*timer_callback)(evutil_socket_t fd, short what, void *arg);

/**
 * @brief Register a one-shot timer (automatically destroyed after single trigger)
 * @param sec Timeout in seconds
 * @param usec Timeout in microseconds (0~999999)
 * @param cb Callback function invoked when timer expires
 * @param arg User context pointer passed to the callback
 * @return Pointer to the allocated event struct on success, NULL on failure
 * 
 * @note This timer does not require manual cleanup - it will be automatically
 *       freed after triggering. If you need to cancel it before expiration,
 *       call `unregister_timer()`.
 * 
 * Example usage (replaces `sleep(5)` for delayed task creation):
 * @code
 * struct event *timer = register_one_shot_timer(5, 0, on_delayed_task, "test_task");
 * @endcode
 */
struct event* register_one_shot_timer(unsigned int sec, unsigned int usec,
                                     timer_callback cb, void *arg);

/**
 * @brief Register a periodic timer (triggers repeatedly at fixed intervals)
 * @param sec Interval in seconds
 * @param usec Interval in microseconds (0~999999)
 * @param cb Callback function invoked on each trigger
 * @param arg User context pointer passed to the callback
 * @return Pointer to the allocated event struct on success, NULL on failure
 * 
 * @note Periodic timers must be manually canceled via `unregister_timer()`
 *       when no longer needed (e.g., during service shutdown or connection loss).
 *       They will otherwise persist until the event loop terminates.
 * 
 * Example usage (30-second heartbeat reporting):
 * @code
 * struct event *hb_timer = register_periodic_timer(30, 0, on_heartbeat, NULL);
 * @endcode
 */
struct event* register_periodic_timer(unsigned int sec, unsigned int usec,
                                     timer_callback cb, void *arg);

/**
 * @brief Cancel and destroy a registered timer
 * @param timer Pointer to the timer event returned by `register_*_timer()`
 * 
 * @note After calling this function, the `timer` pointer becomes invalid and
 *       must not be used again. Safe to call even if the timer has already expired
 *       (for one-shot timers) or is pending execution.
 */
void unregister_timer(struct event *timer);

#ifdef __cplusplus
}
#endif

#endif // EVENT_TIMER_H