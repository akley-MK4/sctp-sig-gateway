#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <event2/event.h>

/**
 * User callback invoked when a registered signal is delivered.
 * @param signum  The signal number (e.g. SIGTERM, SIGINT)
 * @param arg     Opaque pointer passed at registration
 */
typedef void (*signal_cb)(int signum, void *arg);

/**
 * Register a callback for a signal. Uses libevent's persistent signal events.
 * If the signal is already registered, the old entry is replaced.
 * @return 0 on success, 1 on failure
 */
int register_signal_handler(struct event_base *eb, int signum, signal_cb cb, void *arg);

/**
 * Unregister a signal handler.
 * @return 0 on success, 1 if not found
 */
int unregister_signal_handler(int signum);

/**
 * Shut down all signal events and release resources.
 */
void signal_handler_shutdown(void);

#ifdef __cplusplus
}
#endif

/* ================================================================ */
/*  C++ class definition — invisible to C compiler                  */
/* ================================================================ */
#ifdef __cplusplus

#include <map>
#include <mutex>
#include <event2/event.h>

class SignalHandler {
public:
    SignalHandler(const SignalHandler&)            = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    SignalHandler(SignalHandler&&)                 = delete;
    SignalHandler& operator=(SignalHandler&&)      = delete;

    static SignalHandler& getInstance();

    int  register_sig(struct event_base *eb, int signum, signal_cb cb, void *arg);
    int  unregister_sig(int signum);
    void shutdown();

private:
    struct SigInfo {
        struct event_base *eb_;
        signal_cb          cb_;
        void              *arg_;
        struct event      *ev_;
    };

    SignalHandler();
    ~SignalHandler();

    static void ev_sig_cb(evutil_socket_t sig, short events, void *arg);

    std::map<int, SigInfo> sig_map_;
    std::mutex              mtx_;
};

#endif /* __cplusplus */
#endif /* SIGNAL_HANDLER_H */