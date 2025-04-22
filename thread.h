#ifndef __LLVMTOOL_LACHESIS_THREAD__
#define __LLVMTOOL_LACHESIS_THREAD__
#include <tool.h>

void stacktrace_add_stackframe(char * function_name);
void stacktrace_remove_stackframe();

void stacktrace_add_location_info(char * file, int32_t line);
void stacktrace_remove_location_info();

void add_thread_info(std::thread::id tid, char * file,int32_t line);

Backtrace get_stacktrace_copy(THREADID TID);

std::string get_thread_create_location(THREADID TID);
#endif //__LLVMTOOL_LACHESIS_THREAD__