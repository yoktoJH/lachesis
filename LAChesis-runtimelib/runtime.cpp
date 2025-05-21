#include "runtime.h"
#include "tool.h"

#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <map>

static std::mutex tid_map_mutex;
static std::map<std::thread::id, THREADID> realid_to_tid;
static std::map<THREADID, std::thread::id> tid_to_realid;
static std::mutex lock_map_mutex;
static std::map<void *,int64_t> lock_index;
static int64_t lock_count = 0;

static std::map<memory_operation_type, callbacks_storage<std::unique_ptr<memory_callback>>> memory_callbacks;
static std::map<lock_operation_type,callbacks_storage<LOCKFUNPTR>> lock_callbacks;
static std::vector<THREADFUNPTR> thread_create_callbacks;
static std::vector<FORKFUNPTR> thread_forked_callbacks;
callbacks_storage<JOINFUNPTR> join_callbacks;


void register_before_memory_callback(std::unique_ptr<memory_callback> callback, const memory_operation_type op) {
    memory_callbacks[op].before.emplace_back(std::move(callback));
}

void register_after_memory_callback(std::unique_ptr<memory_callback> callback, const memory_operation_type op) {
    memory_callbacks[op].after.emplace_back(std::move(callback));
}

void call_memory_callbacks(memory_operation_type op, bool is_before, ADDRINT addr, UINT32 size, const char *var_name,
                           const char *var_type, UINT32 var_offset, const char *loc_file, INT32 loc_line, BOOL is_local,
                           ADDRINT ins) {
    const VARIABLE variable = {var_name, var_type, var_offset};
    const LOCATION location = {loc_file, loc_line};

    const THREADID tid = get_THREADID(std::this_thread::get_id());

    auto callbacks = &callbacks_storage<std::unique_ptr<memory_callback>>::after;

    if (is_before) {
        callbacks = &callbacks_storage<std::unique_ptr<memory_callback>>::before;
    }

    for (const auto &callback: memory_callbacks[op].*callbacks) {
        callback->call(tid, addr, size, variable, location, is_local, ins);
    }
}



void register_thread_create_callback(THREADFUNPTR callback) {
    thread_create_callbacks.push_back(callback);
}

void call_thread_create_callbacks(THREADID TID) {
    for (const auto callback:thread_create_callbacks) {
        callback(TID);
    }
}

void call_thread_create_callbacks() {
    call_thread_create_callbacks(get_THREADID(std::this_thread::get_id()));
}


void register_before_lock_callback(LOCKFUNPTR callback,const lock_operation_type op) {
    lock_callbacks[op].before.emplace_back(callback);
}

void register_after_lock_callback(LOCKFUNPTR callback,const lock_operation_type op) {
    lock_callbacks[op].after.emplace_back(callback);
}

void call_lock_callbacks(void * lock_ptr,const lock_operation_type op,const bool is_before) {

    LOCK lock;
    lock.q_set(lock_ptr_to_index(lock_ptr));
    const THREADID tid = get_THREADID(std::this_thread::get_id());

    auto callbacks = &callbacks_storage<LOCKFUNPTR>::after;

    if (is_before) {
        callbacks = &callbacks_storage<LOCKFUNPTR>::before;
    }

    for (const auto &callback: lock_callbacks[op].*callbacks) {
        callback(tid,lock);
    }
}

THREADID get_THREADID(const std::thread::id realid) {
    std::lock_guard lock(tid_map_mutex);
    if (const auto tid_i = realid_to_tid.find(realid); tid_i != realid_to_tid.end()) {
        return tid_i->second;
    }

    for (THREADID i = 0; i < UINT32_MAX; i++) {
        if (tid_to_realid.find(i) == tid_to_realid.end()) {
            realid_to_tid[realid] = i;
            tid_to_realid[i] = realid;
            return i;
        }
    }
    std::cerr << "no free THREADID, there is no way to recover from this\n";
    exit(1);
}

std::thread::id THREADID_to_tid(THREADID id) {
    std::lock_guard lock(tid_map_mutex);
    if (const auto tid_i = tid_to_realid.find(id); tid_i != tid_to_realid.end()) {
        return tid_i->second;
    }
    std::cerr << "unknown THREADID" << id << std::endl;
    exit(1);
}

int64_t lock_ptr_to_index(void * lock_ptr) {
    std::lock_guard lock(lock_map_mutex);
    if (const auto iter = lock_index.find(lock_ptr);iter == lock_index.end()) {
        lock_index[lock_ptr] = lock_count++;

    }
    return  lock_index[lock_ptr];
}

void register_thread_forked_callback(FORKFUNPTR callback) {
    thread_forked_callbacks.push_back(callback);
}
void call_thread_forked_callbacks(THREADID nTID){
    THREADID TID = get_THREADID(std::this_thread::get_id());
    for (const auto callback:thread_forked_callbacks) {
        callback(TID,nTID);
    }
}


void register_before_join_callback(JOINFUNPTR callback) {
    join_callbacks.before.push_back(callback);
}
void register_after_join_callback(JOINFUNPTR callback) {
    join_callbacks.after.push_back(callback);
}
void call_join_callbacks(pthread_t thread, bool is_before) {
    THREADID jTID = get_THREADID(std::thread::id(thread));
    THREADID TID = get_THREADID(std::this_thread::get_id());
    auto callbacks = &callbacks_storage<JOINFUNPTR>::after;

    if (is_before) {
        callbacks = &callbacks_storage<JOINFUNPTR>::before;
    }
    for (JOINFUNPTR callback:join_callbacks.*callbacks) {
        callback(TID,jTID);
    }
}