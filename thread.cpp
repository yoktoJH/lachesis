#include <map>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>

#include "types.h"
#include "runtime.h"
#include "thread.h"

static char no_file[]={' '};

struct thread_info {
    std::vector<stack_frame> stacktrace;
    // file and line where thread was created
    char *file_name = no_file;
    int32_t line = -1;
    std::optional<THREADID> parent_thread;
    std::mutex thread_start_mutex;
    std::condition_variable cv;
    bool callbacks_called = false;
};

static std::mutex thread_info_mutex;
static std::map<THREADID, thread_info> thread_map;

void add_thread_info(const std::thread::id ntid, char *file, int32_t line) {
    const THREADID nTID = get_THREADID(ntid);
    const THREADID TID = get_THREADID(std::this_thread::get_id());

    {
        std::lock_guard lock(thread_info_mutex);
        thread_info &info = thread_map[nTID];

        info.file_name = file;
        info.line = line;
        info.parent_thread = TID;
    }


    call_thread_create_callbacks(nTID);
    call_thread_forked_callbacks(nTID);
    {
        std::lock_guard lock(thread_info_mutex);
        thread_map[nTID].callbacks_called=true;
        thread_map[nTID].cv.notify_all();
    }
}

void stacktrace_add_stackframe(char *function_name) {
    const THREADID TID = get_THREADID(std::this_thread::get_id());
    {
        std::unique_lock lock(thread_info_mutex);
        thread_info &info = thread_map[TID];
        stack_frame& stackframe = info.stacktrace.emplace_back();
        stackframe.function_name = function_name;
        stackframe.file_name = no_file;
        info.cv.wait(lock,[&info = info, TID = TID]{return TID == 0 || info.callbacks_called==true;});
    }
    if (TID ==0) {
        call_thread_create_callbacks(TID);
    }
}

void stacktrace_remove_stackframe() {
    std::lock_guard lock(thread_info_mutex);
    THREADID TID = get_THREADID(std::this_thread::get_id());
    thread_info &info = thread_map.at(TID);

    if (info.stacktrace.size() == 1) {
        thread_map.erase(TID);
    } else {
        info.stacktrace.pop_back();
    }
}

void stacktrace_add_location_info(char *file, int32_t line) {
    std::lock_guard lock(thread_info_mutex);
    const THREADID TID = get_THREADID(std::this_thread::get_id());
    thread_info &info = thread_map.at(TID);

    info.stacktrace.back().file_name = file;
    info.stacktrace.back().line = line;
}

void stacktrace_remove_location_info() {
    std::lock_guard lock(thread_info_mutex);
    const THREADID TID = get_THREADID(std::this_thread::get_id());
    thread_info &info = thread_map.at(TID);

    info.stacktrace.back().file_name = no_file;
    info.stacktrace.back().line = -1;
}

Backtrace get_stacktrace_copy(THREADID TID) {
    std::lock_guard lock(thread_info_mutex);
    return thread_map.at(TID).stacktrace;
}

std::string get_thread_create_location(THREADID TID) {
    std::lock_guard lock(thread_info_mutex);
    thread_info & info = thread_map.at(TID);
    return std::string(info.file_name)+":"+std::to_string(info.line);
}