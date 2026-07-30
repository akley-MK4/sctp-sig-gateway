#include "file_watcher.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* -------------------------------------------------------------- */
/*  Singleton instance                                             */
/* -------------------------------------------------------------- */

FileWatcher& FileWatcher::getInstance() {
    static FileWatcher instance;
    return instance;
}

/* -------------------------------------------------------------- */
/*  Construction / destruction                                     */
/* -------------------------------------------------------------- */

FileWatcher::FileWatcher()
    : inotify_fd_(-1), bev_(nullptr) {}

FileWatcher::~FileWatcher() {
    shutdown();
}

/* -------------------------------------------------------------- */
/*  Private helpers                                                */
/* -------------------------------------------------------------- */

int FileWatcher::init_inotify(struct event_base *eb) {
    if (inotify_fd_ >= 0)
        return 0;

    inotify_fd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (inotify_fd_ < 0) {
        perror("file_watcher: inotify_init1");
        return 1;
    }

    bev_ = bufferevent_socket_new(eb, inotify_fd_, 0);
    if (!bev_) {
        close(inotify_fd_);
        inotify_fd_ = -1;
        return 1;
    }

    bufferevent_setcb(bev_, readcb, nullptr, nullptr, nullptr);
    bufferevent_enable(bev_, EV_READ);
    return 0;
}

/* -------------------------------------------------------------- */
/*  bufferevent read callback                                      */
/* -------------------------------------------------------------- */

void FileWatcher::readcb(struct bufferevent *bev, void * /*arg*/) {
    char buf[4096];
    int num_read = bufferevent_read(bev, buf, sizeof(buf));
    if (num_read <= 0)
        return;

    char *p = buf;
    while (p < buf + num_read) {
        auto *ev = reinterpret_cast<struct inotify_event *>(p);
        FileWatcher::getInstance().handle_event(ev);
        p += sizeof(struct inotify_event) + ev->len;
    }
}

/* -------------------------------------------------------------- */
/*  Per-event handler: rebuild watch + invoke user callback        */
/* -------------------------------------------------------------- */

void FileWatcher::handle_event(struct inotify_event *ev) {
    CbInfo info;
    info.cb_ = nullptr;

    {   /* Lock scope: copy out CbInfo only */
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = wd_to_cb_.find(ev->wd);
        if (it != wd_to_cb_.end())
            info = it->second;
    }   /* Unlock */

    if (!info.cb_)
        return;

    /* Rebuild watch and dispatch — no lock held */
    unregister_watch(info.file_.c_str());
    register_watch(info.eb_, info.file_.c_str(), info.cb_, info.arg_);
    info.cb_(info.arg_);
}

/* -------------------------------------------------------------- */
/*  Public member functions                                         */
/* -------------------------------------------------------------- */

int FileWatcher::register_watch(struct event_base *eb, const char *file,
                                 watcher_cb cb, void *arg) {
    if (!eb || !file || !cb)
        return 1;

    std::lock_guard<std::mutex> lock(mtx_);

    if (init_inotify(eb) != 0)
        return 1;

    /* Replace existing watch if already registered */
    auto it = file_to_wd_.find(file);
    if (it != file_to_wd_.end()) {
        inotify_rm_watch(inotify_fd_, it->second);
        wd_to_cb_.erase(it->second);
    }

    int wd = inotify_add_watch(inotify_fd_, file,
                                IN_MODIFY | IN_CLOSE_WRITE | IN_DELETE_SELF);
    if (wd < 0) {
        perror("file_watcher: inotify_add_watch");
        return 1;
    }

    file_to_wd_[file] = wd;
    wd_to_cb_[wd]     = {eb, cb, arg, file};   /* aggregate-init, order matches CbInfo */
    return 0;
}

int FileWatcher::unregister_watch(const char *file) {
    if (!file)
        return 1;

    std::lock_guard<std::mutex> lock(mtx_);

    auto it = file_to_wd_.find(file);
    if (it == file_to_wd_.end())
        return 1;

    inotify_rm_watch(inotify_fd_, it->second);
    wd_to_cb_.erase(it->second);
    file_to_wd_.erase(it);
    return 0;
}

void FileWatcher::shutdown() {
    std::lock_guard<std::mutex> lock(mtx_);

    for (const auto &kv : wd_to_cb_)
        inotify_rm_watch(inotify_fd_, kv.first);

    wd_to_cb_.clear();
    file_to_wd_.clear();

    if (bev_) {
        bufferevent_free(bev_);
        bev_ = nullptr;
    }
    if (inotify_fd_ >= 0) {
        close(inotify_fd_);
        inotify_fd_ = -1;
    }
}

/* -------------------------------------------------------------- */
/*  C API — thin wrappers delegating to the singleton              */
/* -------------------------------------------------------------- */

int register_file_watcher(struct event_base *eb, const char *file,
                           watcher_cb cb, void *arg) {
    return FileWatcher::getInstance().register_watch(eb, file, cb, arg);
}

int unregister_file_watcher(const char *file) {
    return FileWatcher::getInstance().unregister_watch(file);
}

void file_watcher_shutdown(void) {
    FileWatcher::getInstance().shutdown();
}