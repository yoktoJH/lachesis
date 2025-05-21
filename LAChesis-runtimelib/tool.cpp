#include "tool.h"
#include "runtime.h"
#include "thread.h"
#include "callback.h"
//---------------------------------------------------------------------------------------------------------------------
//Before callbacks
VOID ACCESS_BeforeMemoryRead(MEMREADAVFUNPTR callback) {
    register_before_memory_callback(std::make_unique<memory_callback_impl<MEMREADAVFUNPTR, memory_callback_type::AV> >(callback),
                                  memory_operation_type::READ);
}

VOID ACCESS_BeforeMemoryRead(MEMREADAVLFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVLFUNPTR, memory_callback_type::AVL> >(callback),
        memory_operation_type::READ);
}

VOID ACCESS_BeforeMemoryRead(MEMREADAVOFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVOFUNPTR, memory_callback_type::AVO> >(callback),
        memory_operation_type::READ);
}

VOID ACCESS_BeforeMemoryRead(MEMREADAVIOFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVIOFUNPTR, memory_callback_type::AVIO> >(callback),
        memory_operation_type::READ);
}


VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVFUNPTR, memory_callback_type::AV> >(callback),
        memory_operation_type::WRITE);
}

VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVLFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVLFUNPTR, memory_callback_type::AVL> >(callback),
        memory_operation_type::WRITE);
}

VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVOFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVOFUNPTR, memory_callback_type::AVO> >(callback),
        memory_operation_type::WRITE);
}

VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVIOFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVIOFUNPTR, memory_callback_type::AVIO> >(callback),
        memory_operation_type::WRITE);
}


VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVFUNPTR, memory_callback_type::AV> >(callback),
        memory_operation_type::UPDATE);
}

VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVLFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVLFUNPTR, memory_callback_type::AVL> >(callback),
        memory_operation_type::UPDATE);
}

VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVOFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVOFUNPTR, memory_callback_type::AVO> >(callback),
        memory_operation_type::UPDATE);
}

VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVIOFUNPTR callback) {
    register_before_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVIOFUNPTR, memory_callback_type::AVIO> >(callback),
        memory_operation_type::UPDATE);
}

//---------------------------------------------------------------------------------------------------------------------
//After callbacks
VOID ACCESS_AfterMemoryRead(MEMREADAVFUNPTR callback) {
    register_after_memory_callback(std::make_unique<memory_callback_impl<MEMREADAVFUNPTR, memory_callback_type::AV> >(callback),
                                  memory_operation_type::READ);
}

VOID ACCESS_AfterMemoryRead(MEMREADAVLFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVLFUNPTR, memory_callback_type::AVL> >(callback),
        memory_operation_type::READ);
}

VOID ACCESS_AfterMemoryRead(MEMREADAVOFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVOFUNPTR, memory_callback_type::AVO> >(callback),
        memory_operation_type::READ);
}

VOID ACCESS_AfterMemoryRead(MEMREADAVIOFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMREADAVIOFUNPTR, memory_callback_type::AVIO> >(callback),
        memory_operation_type::READ);
}


VOID ACCESS_AfterMemoryWrite(MEMWRITEAVFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVFUNPTR, memory_callback_type::AV> >(callback),
        memory_operation_type::WRITE);
}

VOID ACCESS_AfterMemoryWrite(MEMWRITEAVLFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVLFUNPTR, memory_callback_type::AVL> >(callback),
        memory_operation_type::WRITE);
}

VOID ACCESS_AfterMemoryWrite(MEMWRITEAVOFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVOFUNPTR, memory_callback_type::AVO> >(callback),
        memory_operation_type::WRITE);
}

VOID ACCESS_AfterMemoryWrite(MEMWRITEAVIOFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMWRITEAVIOFUNPTR, memory_callback_type::AVIO> >(callback),
        memory_operation_type::WRITE);
}


VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVFUNPTR callback) {
    register_after_memory_callback(std::make_unique<memory_callback_impl<MEMUPDATEAVFUNPTR, memory_callback_type::AV> >(callback),
                                  memory_operation_type::UPDATE);
}

VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVLFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVLFUNPTR, memory_callback_type::AVL> >(callback),
        memory_operation_type::UPDATE);
}

VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVOFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVOFUNPTR, memory_callback_type::AVO> >(callback),
        memory_operation_type::UPDATE);
}

VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVIOFUNPTR callback) {
    register_after_memory_callback(
        std::make_unique<memory_callback_impl<MEMUPDATEAVIOFUNPTR, memory_callback_type::AVIO> >(callback),
        memory_operation_type::UPDATE);
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
    register_before_lock_callback(callback,lock_operation_type::LOCK);
}
VOID SYNC_BeforeLockRelease(LOCKFUNPTR callback) {
    register_before_lock_callback(callback,lock_operation_type::UNLOCK);
}

VOID SYNC_AfterLockAcquire(LOCKFUNPTR callback) {
    register_after_lock_callback(callback,lock_operation_type::LOCK);
}
VOID SYNC_AfterLockRelease(LOCKFUNPTR callback) {
    register_after_lock_callback(callback,lock_operation_type::UNLOCK);
}
VOID THREAD_ThreadForked(FORKFUNPTR callback) {
    register_thread_forked_callback(callback);
}
VOID SYNC_AfterJoin(JOINFUNPTR callback) {
    register_after_join_callback(callback);
}
VOID SYNC_BeforeJoin(JOINFUNPTR callback) {
    register_before_join_callback(callback);
}