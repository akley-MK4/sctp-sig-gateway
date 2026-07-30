#include "signal_handler.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------- */
/*  Singleton instance                                             */
/* -------------------------------------------------------------- */

SignalHandler& SignalHandler::getInstance() {
    static SignalHandler instance;
    return instance;
}

/* -------------------------------------------------------------- */
/*  Construction / destruction                                     */
/* -------------------------------------------------------------- */

SignalHandler::SignalHandler() = default;

SignalHandler::~SignalHandler() {
    shutdown();
}

/* -------------------------------------------------------------- */
/*  libevent signal callback — looks up SigInfo and dispatches     */
/* -------------------------------------------------------------- */

void SignalHandler::ev_sig_cb(evutil_socket_t sig, short events, void *arg) {
    (void)events;
    auto *self = static_cast<SignalHandler *>(arg);

    SigInfo info;
    info.cb_ = nullptr;

    {   /* Lock scope: copy out SigInfo */
        std::lock_guard<std::mutex> lock(self->mtx_);
        auto it = self->sig_map_.find(static_cast<int>(sig));
        if (it != self->sig_map_.end())
            info = it->second;
    }   /* Unlock */

    if (info.cb_)
        info.cb_(static_cast<int>(sig), info.arg_);
}

/* -------------------------------------------------------------- */
/*  Public member functions                                         */
/* -------------------------------------------------------------- */

int SignalHandler::register_sig(struct event_base *eb, int signum, signal_cb cb, void *arg) {
    if (!eb || signum <= 0 || !cb)
        return 1;

    std::lock_guard<std::mutex> lock(mtx_);

    /* Replace if already registered */
    auto it = sig_map_.find(signum);
    if (it != sig_map_.end()) {
        event_del(it->second.ev_);
        event_free(it->second.ev_);
    }

    struct event *ev = evsignal_new(eb, signum, ev_sig_cb, this);
    if (!ev) {
        perror("signal_handler: evsignal_new");
        return 1;
    }

    if (event_add(ev, nullptr) < 0) {
        perror("signal_handler: event_add");
        event_free(ev);
        return 1;
    }

    sig_map_[signum] = {eb, cb, arg, ev};
    return 0;
}

int SignalHandler::unregister_sig(int signum) {
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = sig_map_.find(signum);
    if (it == sig_map_.end())
        return 1;

    event_del(it->second.ev_);
    event_free(it->second.ev_);
    sig_map_.erase(it);
    return 0;
}

void SignalHandler::shutdown() {
    std::lock_guard<std::mutex> lock(mtx_);

    for (auto &kv : sig_map_) {
        event_del(kv.second.ev_);
        event_free(kv.second.ev_);
    }
    sig_map_.clear();
}

/* -------------------------------------------------------------- */
/*  C API — thin wrappers delegating to the singleton              */
/* -------------------------------------------------------------- */

int register_signal_handler(struct event_base *eb, int signum, signal_cb cb, void *arg) {
    return SignalHandler::getInstance().register_sig(eb, signum, cb, arg);
}

int unregister_signal_handler(int signum) {
    return SignalHandler::getInstance().unregister_sig(signum);
}

void signal_handler_shutdown(void) {
    SignalHandler::getInstance().shutdown();
}