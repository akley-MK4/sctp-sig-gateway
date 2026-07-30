#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <event2/event.h>

typedef void (*watcher_cb)(void *arg);

/**
 * Register a file watch.
 * @return 0 on success, 1 on failure
 */
int  register_file_watcher(struct event_base *eb, const char *file, watcher_cb cb, void *arg);

/**
 * Remove a file watch.
 * @return 0 on success, 1 if not found
 */
int  unregister_file_watcher(const char *file);

/**
 * Shut down all watches and release resources.
 */
void file_watcher_shutdown(void);

#ifdef __cplusplus
}
#endif

/* ================================================================ */
/*  C++ class definition — invisible to C compiler                  */
/* ================================================================ */
#ifdef __cplusplus

#include <map>
#include <mutex>
#include <string>
#include <event2/bufferevent.h>
#include <sys/inotify.h>

class FileWatcher {
public:
    FileWatcher(const FileWatcher&)            = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;
    FileWatcher(FileWatcher&&)                 = delete;
    FileWatcher& operator=(FileWatcher&&)      = delete;

    static FileWatcher& getInstance();

    int  register_watch(struct event_base *eb, const char *file, watcher_cb cb, void *arg);
    int  unregister_watch(const char *file);
    void shutdown();

private:
    struct CbInfo {
        struct event_base *eb_;
        watcher_cb         cb_;
        void              *arg_;
        std::string        file_;
    };

    FileWatcher();
    ~FileWatcher();

    static void readcb(struct bufferevent *bev, void *arg);
    void         handle_event(struct inotify_event *ev);
    int          init_inotify(struct event_base *eb);

    int                         inotify_fd_;
    struct bufferevent         *bev_;
    std::map<int, CbInfo>       wd_to_cb_;
    std::map<std::string, int>  file_to_wd_;
    std::mutex                  mtx_;
};

#endif /* __cplusplus */
#endif /* FILE_WATCHER_H */