#ifndef __LLVMTOOL_LACHESIS_THREAD__
#define __LLVMTOOL_LACHESIS_THREAD__
#include <tool.h>

static char empty_string[] = {' '};
struct thread_info
{
    std::vector<stack_frame> stacktrace;
    // file and line where thread was created
    char *file_name = empty_string;
    char *func_name = empty_string;
    int32_t line = -1;
    std::optional<THREADID> parent_thread;
};


void stacktrace_add_stackframe(char * function_name);
void stacktrace_remove_stackframe();

void stacktrace_add_location_info(char * file, int32_t line);
void stacktrace_remove_location_info();

void add_thread_info(THREADID newTID, char * file,int32_t line);

Backtrace get_stacktrace_copy(THREADID TID);

std::string get_thread_create_location(THREADID TID);

THREADID tid_to_THREADID(tid_t realid);

thread_info svcomp_get_thread_info(const THREADID TID);

tid_t THREADID_to_tid(THREADID id);
int start_new_thread(pthread_t *thread, const pthread_attr_t *attr,
                     void *(*start_routine)(void *), void *arg, char *file_name, int32_t line);

const tid_t get_this_thread_id();

#endif //__LLVMTOOL_LACHESIS_THREAD__