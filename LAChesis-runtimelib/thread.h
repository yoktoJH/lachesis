#ifndef __LLVMTOOL_LACHESIS_THREAD__
#define __LLVMTOOL_LACHESIS_THREAD__
#include <tool.h>

void stacktrace_add_stackframe(char * function_name);
void stacktrace_remove_stackframe();

void stacktrace_add_location_info(char * file, int32_t line);
void stacktrace_remove_location_info();

void add_thread_info(THREADID newTID, char * file,int32_t line);

Backtrace get_stacktrace_copy(THREADID TID);

std::string get_thread_create_location(THREADID TID);

THREADID tid_to_THREADID(std::thread::id realid);

std::thread::id THREADID_to_tid(THREADID id);
int start_new_thread(pthread_t *thread, const pthread_attr_t *attr,
                     void *(*start_routine)(void *), void *arg, char *file_name, int32_t line);

#endif //__LLVMTOOL_LACHESIS_THREAD__