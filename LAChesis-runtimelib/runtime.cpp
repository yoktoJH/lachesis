#include "runtime.h"
#include "tool.h"
#include "thread.h"

#include <thread>
#include <mutex>
#include <vector>
#include <map>


static std::mutex lock_map_mutex;
static std::map<void *, int64_t> lock_index;
static int64_t lock_count = 0;

static std::map<memory_operation_t, callbacks_storage<std::unique_ptr<memory_callback> > > memory_callbacks;
static std::map<lock_operation_t, callbacks_storage<LOCKFUNPTR> > lock_callbacks;
static std::vector<THREADFUNPTR> thread_create_callbacks;
static std::vector<FORKFUNPTR> thread_forked_callbacks;
callbacks_storage<JOINFUNPTR> join_callbacks;

void register_memory_callback(std::unique_ptr<memory_callback> callback, const memory_operation_t op,
                              const HookPosition position) {
    if (position == HookPosition::Before) {
        memory_callbacks[op].before.emplace_back(std::move(callback));
    } else {
        memory_callbacks[op].after.emplace_back(std::move(callback));
    }
}

void register_before_memory_callback(std::unique_ptr<memory_callback> callback, const memory_operation_t op) {
    memory_callbacks[op].before.emplace_back(std::move(callback));
}

void register_after_memory_callback(std::unique_ptr<memory_callback> callback, const memory_operation_t op) {
    memory_callbacks[op].after.emplace_back(std::move(callback));
}

void call_memory_callbacks(memory_operation_t op, HookPosition position, ADDRINT addr, UINT32 size,
                           const char *var_name,
                           const char *var_type, UINT32 var_offset, const char *loc_file, INT32 loc_line, BOOL is_local,
                           ADDRINT ins) {
    const VARIABLE variable = {var_name, var_type, var_offset};
    const LOCATION location = {loc_file, loc_line};

    const THREADID TID = tid_to_THREADID(std::this_thread::get_id());

    auto callbacks = &callbacks_storage<std::unique_ptr<memory_callback> >::after;

    if (position == HookPosition::Before) {
        callbacks = &callbacks_storage<std::unique_ptr<memory_callback> >::before;
    }

    for (const auto &callback: memory_callbacks[op].*callbacks) {
        callback->call(TID, addr, size, variable, location, is_local, ins);
    }
}


void register_thread_create_callback(THREADFUNPTR callback) {
    thread_create_callbacks.push_back(callback);
}

void call_thread_create_callbacks(THREADID TID) {
    for (const auto callback: thread_create_callbacks) {
        callback(TID);
    }
}

void call_thread_create_callbacks() {
    call_thread_create_callbacks(tid_to_THREADID(std::this_thread::get_id()));
}


void register_lock_callback(LOCKFUNPTR callback, const lock_operation_t op, const HookPosition position) {
    if (position == HookPosition::Before) {
        lock_callbacks[op].before.emplace_back(callback);
    } else {
        lock_callbacks[op].after.emplace_back(callback);
    }
}

void register_before_lock_callback(LOCKFUNPTR callback, const lock_operation_t op) {
    lock_callbacks[op].before.emplace_back(callback);
}

void register_after_lock_callback(LOCKFUNPTR callback, const lock_operation_t op) {
    lock_callbacks[op].after.emplace_back(callback);
}

void call_lock_callbacks(void *lock_ptr, const lock_operation_t op, HookPosition position) {
    LOCK lock;
    lock.q_set(lock_ptr_to_index(lock_ptr));
    const THREADID tid = tid_to_THREADID(std::this_thread::get_id());

    auto callbacks = &callbacks_storage<LOCKFUNPTR>::after;

    if (position == HookPosition::Before) {
        callbacks = &callbacks_storage<LOCKFUNPTR>::before;
    }

    for (const auto &callback: lock_callbacks[op].*callbacks) {
        callback(tid, lock);
    }
}


int64_t lock_ptr_to_index(void *lock_ptr) {
    std::lock_guard lock(lock_map_mutex);
    if (const auto iter = lock_index.find(lock_ptr); iter == lock_index.end()) {
        lock_index[lock_ptr] = lock_count++;
    }
    return lock_index[lock_ptr];
}

void register_thread_forked_callback(FORKFUNPTR callback) {
    thread_forked_callbacks.push_back(callback);
}

void call_thread_forked_callbacks(THREADID newTID) {
    THREADID TID = tid_to_THREADID(std::this_thread::get_id());
    for (const auto callback: thread_forked_callbacks) {
        callback(TID, newTID);
    }
}


void register_join_callback(JOINFUNPTR callback, const HookPosition position) {
    if (position == HookPosition::Before) {
        join_callbacks.before.push_back(callback);
    } else {
        join_callbacks.after.push_back(callback);
    }
}


void call_join_callbacks(THREADID joinedTID, HookPosition position) {
    THREADID TID = tid_to_THREADID(std::this_thread::get_id());
    auto callbacks = &callbacks_storage<JOINFUNPTR>::after;

    if (position == HookPosition::Before) {
        callbacks = &callbacks_storage<JOINFUNPTR>::before;
    }
    for (const JOINFUNPTR callback: join_callbacks.*callbacks) {
        callback(TID, joinedTID);
    }
}
