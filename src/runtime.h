#ifndef __LLVMTOOL_LACHESIS_RUNTIME__
#define __LLVMTOOL_LACHESIS_RUNTIME__
#include "callback.h"
#include <memory>
#include <tool.h>

void register_memory_callback(std::unique_ptr<memory_callback> callback, memory_operation_t op, HookPosition position);

void call_memory_callbacks(memory_operation_t op, HookPosition position, ADDRINT addr, UINT32 size, const char *var_name,
                           const char *var_type, UINT32 var_offset, const char *loc_file, INT32 loc_line, BOOL is_local,
                           ADDRINT ins);
void register_thread_create_callback(THREADFUNPTR callback);

void call_thread_create_callbacks(THREADID TID);
//call with current thread
void call_thread_create_callbacks();

void register_thread_forked_callback(FORKFUNPTR callback);
void call_thread_forked_callbacks(THREADID newTID);

void register_lock_callback(LOCKFUNPTR callback, lock_operation_t op, HookPosition position);
void call_lock_callbacks(void * lock, lock_operation_t op, HookPosition position);

void register_join_callback(JOINFUNPTR callback, HookPosition position) ;
void call_join_callbacks(THREADID joinedTID, HookPosition position);


int64_t lock_ptr_to_index(void * lock_ptr);
#endif //__LLVMTOOL_LACHESIS_RUNTIME__
