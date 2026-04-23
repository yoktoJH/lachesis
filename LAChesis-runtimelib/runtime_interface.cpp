#include "runtime.h"
#include "noise.h"
#include "thread.h"
#include "analysers/analysers.h"
#include "config.h"

// TODO make this a header file?
extern "C"
{
    void lachesis_before_read(ADDRINT addr, uint32_t size, char *var_name, char *var_type, uint32_t var_offset,
                              char *loc_file, int32_t loc_line, bool is_local, ADDRINT ins)
    {
        call_memory_callbacks(memory_operation_t::READ, HookPosition::Before, addr, size, var_name, var_type, var_offset,
                              loc_file,
                              loc_line, is_local, ins);
        insert_noise<memory_operation_t::READ>(var_name, loc_line);
    }

    void lachesis_before_write(ADDRINT addr, uint32_t size, char *var_name, char *var_type, uint32_t var_offset,
                               char *loc_file, int32_t loc_line, BOOL is_local, ADDRINT ins)
    {
        call_memory_callbacks(memory_operation_t::WRITE, HookPosition::Before, addr, size, var_name, var_type,
                              var_offset, loc_file,
                              loc_line, is_local, ins);
        insert_noise<memory_operation_t::WRITE>(var_name, loc_line);
    }

    void lachesis_after_read(ADDRINT addr, uint32_t size, char *var_name, char *var_type, uint32_t var_offset,
                             char *loc_file, int32_t loc_line, BOOL is_local, ADDRINT ins)
    {
        call_memory_callbacks(memory_operation_t::READ, HookPosition::After, addr, size, var_name, var_type, var_offset,
                              loc_file,
                              loc_line, is_local, ins);
    }

    void lachesis_after_write(ADDRINT addr, uint32_t size, char *var_name, char *var_type, uint32_t var_offset,
                              char *loc_file, int32_t loc_line, BOOL is_local, ADDRINT ins)
    {
        call_memory_callbacks(memory_operation_t::WRITE, HookPosition::After, addr, size, var_name, var_type, var_offset,
                              loc_file,
                              loc_line, is_local, ins);
    }

    void lachesis_step_out_of_function()
    {
        stacktrace_remove_stackframe();
    }

    void lachesis_step_into_function(char *func_name)
    {
        stacktrace_add_stackframe(func_name);
    }

    // TODO make this function not pthread specific
    int lachesis_thread_create(pthread_t *thread, const pthread_attr_t *attr,  void *(*start_routine)(void *),
                               void *arg, char *loc_file, int32_t loc_line)
    {
        return start_new_thread(thread, attr, start_routine, arg, loc_file, loc_line); // add real values here;
        /*if (retval == 0) {
            const std::thread::id ntid(*thread);
            const THREADID newTID = tid_to_THREADID(ntid);
            add_thread_info(newTID, loc_file, loc_line);

            call_thread_forked_callbacks(newTID);
        }*/
    }

    void lachesis_before_call(char *loc_file, int32_t loc_line)
    {
        stacktrace_add_location_info(loc_file, loc_line);
    }

    void lachesis_after_call()
    {
        stacktrace_remove_location_info();
    }

    // TODO make cmake configurable
    void lachesis_init()
    {
        load_config();
        atomrace_init();
        // eraser_init();
        // fasttrack_init();
    }

    // TODO make cmake configurable
    void lachesis_terminate()
    {
        // eraser_terminate();
        // fasttrack_terminate();
    }

    // TODO prototype of trylock wrapper
    int lachesis_mutex_trylock(pthread_mutex_t *mutex)
    {
        int retval = pthread_mutex_trylock(mutex);
        if (retval == 0)
        {
            // before lock call
            // afterlock call
        }
        return retval;
    }

    int lachesis_rwlock_tryrdlock(pthread_rwlock_t *lock)
    {
        int retval = pthread_rwlock_tryrdlock(lock);
        if (retval == 0)
        {
            // before lock call
            // afterlock call
        }
        return retval;
    }

    // TODO
    int lachesis_rwlock_trywrlock(pthread_rwlock_t *lock)
    {
        int retval = pthread_rwlock_trywrlock(lock);
        if (retval == 0)
        {
            // before lock call
            // afterlock call
        }
        return retval;
    }

    // write lock is currently unused as for now the tool does not differentiate between lock types
    void lachesis_before_lock(void *lock_ptr, __attribute_maybe_unused__ bool is_write_lock)
    {
        call_lock_callbacks(lock_ptr, lock_operation_t::LOCK, HookPosition::Before);
    }

    void lachesis_after_lock(void *lock_ptr, __attribute_maybe_unused__ bool is_write_lock)
    {
        call_lock_callbacks(lock_ptr, lock_operation_t::LOCK, HookPosition::After);
    }

    void lachesis_before_unlock(void *lock_ptr)
    {
        call_lock_callbacks(lock_ptr, lock_operation_t::UNLOCK, HookPosition::Before);
    }

    void lachesis_after_unlock(void *lock_ptr)
    {
        call_lock_callbacks(lock_ptr, lock_operation_t::UNLOCK, HookPosition::After);
    }

    void lachesis_before_join(pthread_t newthread)
    {
        call_join_callbacks(tid_to_THREADID(std::thread::id(newthread)), HookPosition::Before);
    }

    void lachesis_after_join(pthread_t newthread)
    {
        call_join_callbacks(tid_to_THREADID(std::thread::id(newthread)), HookPosition::After);
    }
}
