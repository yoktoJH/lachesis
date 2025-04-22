#ifndef __LLVMTOOL_LACHESIS_RUNTIME__
#define __LLVMTOOL_LACHESIS_RUNTIME__
#include "callback.h"
#include <memory>
#include <thread>
#include <tool.h>

#define BEFORE true
#define AFTER false

void register_before_memory_callback(std::unique_ptr<memory_callback> callback, memory_operation_type op);

void register_after_memory_callback(std::unique_ptr<memory_callback> callback, memory_operation_type op);

void call_memory_callbacks(memory_operation_type op, bool is_before, ADDRINT addr, UINT32 size, const char *var_name,
                           const char *var_type, UINT32 var_offset, const char *loc_file, INT32 loc_line, BOOL is_local,
                           ADDRINT ins);
void register_thread_create_callback(THREADFUNPTR callback);

void call_thread_create_callbacks(THREADID TID);
//call with current thread
void call_thread_create_callbacks();

void register_thread_forked_callback(FORKFUNPTR callback);
void call_thread_forked_callbacks(THREADID nTID);


void register_before_lock_callback(LOCKFUNPTR callback, lock_operation_type op);
void register_after_lock_callback(LOCKFUNPTR callback, lock_operation_type op) ;
void call_lock_callbacks(void * lock, lock_operation_type op, bool is_before);

void register_before_join_callback(JOINFUNPTR callback);
void register_after_join_callback(JOINFUNPTR callback);
void call_join_callbacks(pthread_t thread, bool is_before);

std::thread::id THREADID_to_tid(THREADID id);

THREADID get_THREADID(std::thread::id realid);

int64_t lock_ptr_to_index(void * lock_ptr);
#endif //__LLVMTOOL_LACHESIS_RUNTIME__
