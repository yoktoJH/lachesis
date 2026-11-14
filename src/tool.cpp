#include "tool.h"
#include "runtime.h"
#include "thread.h"
#include "callback.h"
#include "svcomp/witness.h"
#include <vector>
//---------------------------------------------------------------------------------------------------------------------
//Before callbacks
VOID ACCESS_BeforeMemoryRead(MEMREADAVFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVFUNPTR, memory_callback_t::AV> >(callback),
        memory_operation_t::READ, HookPosition::Before);
}

VOID ACCESS_BeforeMemoryRead(MEMREADAVLFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVLFUNPTR, memory_callback_t::AVL> >(callback),
        memory_operation_t::READ, HookPosition::Before);
}

VOID ACCESS_BeforeMemoryRead(MEMREADAVOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVOFUNPTR, memory_callback_t::AVO> >(callback),
        memory_operation_t::READ, HookPosition::Before);
}

VOID ACCESS_BeforeMemoryRead(MEMREADAVIOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVIOFUNPTR, memory_callback_t::AVIO> >(callback),
        memory_operation_t::READ, HookPosition::Before);
}


VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVFUNPTR, memory_callback_t::AV> >(callback),
        memory_operation_t::WRITE, HookPosition::Before);
}

VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVLFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVLFUNPTR, memory_callback_t::AVL> >(callback),
        memory_operation_t::WRITE, HookPosition::Before);
}

VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVOFUNPTR, memory_callback_t::AVO> >(callback),
        memory_operation_t::WRITE, HookPosition::Before);
}

VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVIOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVIOFUNPTR, memory_callback_t::AVIO> >(callback),
        memory_operation_t::WRITE, HookPosition::Before);
}


VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVFUNPTR, memory_callback_t::AV> >(callback),
        memory_operation_t::UPDATE, HookPosition::Before);
}

VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVLFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVLFUNPTR, memory_callback_t::AVL> >(callback),
        memory_operation_t::UPDATE, HookPosition::Before);
}

VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVOFUNPTR, memory_callback_t::AVO> >(callback),
        memory_operation_t::UPDATE, HookPosition::Before);
}

VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVIOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVIOFUNPTR, memory_callback_t::AVIO> >(callback),
        memory_operation_t::UPDATE, HookPosition::Before);
}

//---------------------------------------------------------------------------------------------------------------------
//After callbacks
VOID ACCESS_AfterMemoryRead(MEMREADAVFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVFUNPTR, memory_callback_t::AV> >(callback),
        memory_operation_t::READ, HookPosition::After);
}

VOID ACCESS_AfterMemoryRead(MEMREADAVLFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVLFUNPTR, memory_callback_t::AVL> >(callback),
        memory_operation_t::READ, HookPosition::After);
}

VOID ACCESS_AfterMemoryRead(MEMREADAVOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVOFUNPTR, memory_callback_t::AVO> >(callback),
        memory_operation_t::READ, HookPosition::After);
}

VOID ACCESS_AfterMemoryRead(MEMREADAVIOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVIOFUNPTR, memory_callback_t::AVIO> >(callback),
        memory_operation_t::READ, HookPosition::After);
}


VOID ACCESS_AfterMemoryWrite(MEMWRITEAVFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVFUNPTR, memory_callback_t::AV> >(callback),
        memory_operation_t::WRITE, HookPosition::After);
}

VOID ACCESS_AfterMemoryWrite(MEMWRITEAVLFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVLFUNPTR, memory_callback_t::AVL> >(callback),
        memory_operation_t::WRITE, HookPosition::After);
}

VOID ACCESS_AfterMemoryWrite(MEMWRITEAVOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVOFUNPTR, memory_callback_t::AVO> >(callback),
        memory_operation_t::WRITE, HookPosition::After);
}

VOID ACCESS_AfterMemoryWrite(MEMWRITEAVIOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVIOFUNPTR, memory_callback_t::AVIO> >(callback),
        memory_operation_t::WRITE, HookPosition::After);
}


VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVFUNPTR, memory_callback_t::AV> >(callback),
        memory_operation_t::UPDATE, HookPosition::After);
}

VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVLFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVLFUNPTR, memory_callback_t::AVL> >(callback),
        memory_operation_t::UPDATE, HookPosition::After);
}

VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVOFUNPTR, memory_callback_t::AVO> >(callback),
        memory_operation_t::UPDATE, HookPosition::After);
}

VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVIOFUNPTR callback) {
    register_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVIOFUNPTR, memory_callback_t::AVIO> >(callback),
        memory_operation_t::UPDATE, HookPosition::After);
}

//---------------------------------------------------------------------------------------------------------------------
//Backtrace
VOID THREAD_GetBacktrace(THREADID tid, Backtrace &bt) {
    bt = get_stacktrace_copy(tid);
}

VOID THREAD_GetBacktraceSymbols(Backtrace &bt, Symbols &symbols) {
    //i dont know if i should just implement this in here as the internal structure is obvious and known
    for (auto &stackframe: bt) {
        std::string symbol = std::string(stackframe.function_name) + ":" + std::to_string(stackframe.line) + ", " +
                             std::string(stackframe.file_name);
        symbols.emplace_back(symbol);
    }
}

VOID THREAD_GetThreadCreationLocation(THREADID tid, std::string &location) {
    location = get_thread_create_location(tid);
}

//---------------------------------------------------------------------------------------------------------------------
//Thread
VOID THREAD_ThreadStarted(THREADFUNPTR callback) {
    register_thread_create_callback(callback);
}

VOID SYNC_BeforeLockAcquire(LOCKFUNPTR callback) {
    register_lock_callback(callback, lock_operation_t::LOCK, HookPosition::Before);
}

VOID SYNC_BeforeLockRelease(LOCKFUNPTR callback) {
    register_lock_callback(callback, lock_operation_t::UNLOCK, HookPosition::Before);
}

VOID SYNC_AfterLockAcquire(LOCKFUNPTR callback) {
    register_lock_callback(callback, lock_operation_t::LOCK, HookPosition::After);
}

VOID SYNC_AfterLockRelease(LOCKFUNPTR callback) {
    register_lock_callback(callback, lock_operation_t::UNLOCK, HookPosition::After);
}

VOID THREAD_ThreadForked(FORKFUNPTR callback) {
    register_thread_forked_callback(callback);
}

VOID SYNC_AfterJoin(JOINFUNPTR callback) {
    register_join_callback(callback, HookPosition::After);
}

VOID SYNC_BeforeJoin(JOINFUNPTR callback) {
    register_join_callback(callback, HookPosition::Before);
}

/*************************** SVCOMP ****************************/

AccessData create_access_data(ACCESSREPORT access) 
{
    std::vector<threadData> t_history={};
    thread_info t= svcomp_get_thread_info(access.thread);
    
    AccessData a{
        .variable_data=access,
        .func_name=t.stacktrace.back().function_name,
    };

    while (t.parent_thread.has_value())
    {
        threadData d = {
            .create_location= {t.file_name,t.line,t.column},
            .func_name=t.func_name
        };
        t = svcomp_get_thread_info(t.parent_thread.value());
        t_history.emplace_back(std::move(d));
    }
    
    a.thread_create_history = std::move(t_history);
    return a;
}


VOID REPORT_DataRace(ACCESSREPORT access1, ACCESSREPORT access2){
    
    AccessData a1 = create_access_data(access1);
    AccessData a2 = create_access_data(access2);

    create_witness(a1,a2);
}