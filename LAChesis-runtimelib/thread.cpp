#include <map>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <latch>
#include <pthread.h>

#include "types.h"
#include "runtime.h"
#include "thread.h"


static std::mutex tid_map_mutex;
static std::map<std::thread::id, THREADID> realid_to_tid;
static std::map<THREADID, std::thread::id> tid_to_realid;

THREADID tid_to_THREADID(const std::thread::id realid)
{
    std::lock_guard lock(tid_map_mutex);
    if (const auto tid_i = realid_to_tid.find(realid); tid_i != realid_to_tid.end())
    {
        return tid_i->second;
    }

    for (THREADID i = 0; i < UINT32_MAX; i++)
    {
        if (tid_to_realid.find(i) == tid_to_realid.end())
        {
            realid_to_tid[realid] = i;
            tid_to_realid[i] = realid;
            return i;
        }
    }
    std::cerr << "no free THREADID, there is no way to recover from this\n";
    exit(1);
}

std::thread::id THREADID_to_tid(THREADID id)
{
    std::lock_guard lock(tid_map_mutex);
    if (const auto tid_i = tid_to_realid.find(id); tid_i != tid_to_realid.end())
    {
        return tid_i->second;
    }
    std::cerr << "unknown THREADID" << id << std::endl;
    exit(1);
}

static char no_file[] = {' '};

struct thread_info
{
    std::vector<stack_frame> stacktrace;
    // file and line where thread was created
    char *file_name = no_file;
    int32_t line = -1;
    std::optional<THREADID> parent_thread;
};

static std::mutex thread_info_mutex;
static std::map<THREADID, thread_info> thread_map;

void add_thread_info(const THREADID newTID, char *file, int32_t line)
{
    const THREADID TID = tid_to_THREADID(std::this_thread::get_id());

    {
        std::lock_guard lock(thread_info_mutex);
        thread_info &info = thread_map[newTID];

        info.file_name = file;
        info.line = line;
        info.parent_thread = TID;
    }

}

void stacktrace_add_stackframe(char *function_name)
{
    const THREADID TID = tid_to_THREADID(std::this_thread::get_id());
    {
        std::unique_lock lock(thread_info_mutex);
        thread_info &info = thread_map[TID];
        stack_frame &stackframe = info.stacktrace.emplace_back();
        stackframe.function_name = function_name;
        stackframe.file_name = no_file;
    }
    if (TID == 0)
    {
        call_thread_create_callbacks(TID);
    }
}

void stacktrace_remove_stackframe()
{
    std::lock_guard lock(thread_info_mutex);
    const THREADID TID = tid_to_THREADID(std::this_thread::get_id());

    if (thread_info &info = thread_map.at(TID); info.stacktrace.size() == 1)
    {
        thread_map.erase(TID);
    }
    else
    {
        info.stacktrace.pop_back();
    }
}

void stacktrace_add_location_info(char *file, int32_t line)
{
    std::lock_guard lock(thread_info_mutex);
    const THREADID TID = tid_to_THREADID(std::this_thread::get_id());
    thread_info &info = thread_map.at(TID);

    info.stacktrace.back().file_name = file;
    info.stacktrace.back().line = line;
}

void stacktrace_remove_location_info()
{
    std::lock_guard lock(thread_info_mutex);
    const THREADID TID = tid_to_THREADID(std::this_thread::get_id());
    thread_info &info = thread_map.at(TID);

    info.stacktrace.back().file_name = no_file;
    info.stacktrace.back().line = -1;
}

Backtrace get_stacktrace_copy(const THREADID TID)
{
    std::lock_guard lock(thread_info_mutex);
    return thread_map.at(TID).stacktrace;
}

std::string get_thread_create_location(const THREADID TID)
{
    std::lock_guard lock(thread_info_mutex);
    const thread_info &info = thread_map.at(TID);
    return std::string(info.file_name) + ":" + std::to_string(info.line);
}

struct new_thread_data
{
    std::latch latch{1};
    void *(*start_routine)(void *);
    void *arg;
};

void *new_thread_prelude(void *arg)
{
    new_thread_data *data = static_cast<new_thread_data *>(arg);
    data->latch.wait();
    void *ret = data->start_routine(data->arg);
    delete data;
    return ret;
}

int start_new_thread(pthread_t *thread, const pthread_attr_t *attr,
                     void *(*start_routine)(void *), void *arg, char *file_name, int32_t line)
{
    new_thread_data *data = new new_thread_data;
    data->start_routine = start_routine;
    data->arg = arg;
    int retval = pthread_create(thread, attr, &new_thread_prelude, data);
    if (retval == 0)
    {
        const std::thread::id ntid(*thread);
        const THREADID newTID = tid_to_THREADID(ntid);
        add_thread_info(newTID, file_name, line);
        call_thread_create_callbacks(newTID);
        call_thread_forked_callbacks(newTID);
        data->latch.count_down();
    }
    else
    {
        delete data;
    }
    return retval;
}