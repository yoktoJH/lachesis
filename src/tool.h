#ifndef __LLVMTOOL_LACHESIS_TOOLAPI__
#define __LLVMTOOL_LACHESIS_TOOLAPI__
#include "types.h"

#include <mutex>
#include <vector>

// Definitions of memory-access-related callback functions
typedef VOID (*MEMREADAFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size);
typedef VOID (*MEMREADAVFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                const VARIABLE &variable);
typedef VOID (*MEMREADAVLFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                 const VARIABLE &variable, const LOCATION &location);
typedef VOID (*MEMREADAVOFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                 const VARIABLE &variable, BOOL isLocal);
typedef VOID (*MEMREADAVIOFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                  const VARIABLE &variable, ADDRINT ins, BOOL isLocal);
typedef VOID (*MEMWRITEAFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size);
typedef VOID (*MEMWRITEAVFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                 const VARIABLE &variable);
typedef VOID (*MEMWRITEAVLFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                  const VARIABLE &variable, const LOCATION &location);
typedef VOID (*MEMWRITEAVOFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                  const VARIABLE &variable, BOOL isLocal);
typedef VOID (*MEMWRITEAVIOFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                   const VARIABLE &variable, ADDRINT ins, BOOL isLocal);
typedef VOID (*MEMUPDATEAFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size);
typedef VOID (*MEMUPDATEAVFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                  const VARIABLE &variable);
typedef VOID (*MEMUPDATEAVLFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                   const VARIABLE &variable, const LOCATION &location);
typedef VOID (*MEMUPDATEAVOFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                   const VARIABLE &variable, BOOL isLocal);
typedef VOID (*MEMUPDATEAVIOFUNPTR)(THREADID tid, ADDRINT addr, UINT32 size,
                                    const VARIABLE &variable, ADDRINT ins, BOOL isLocal);

// Functions for registering memory-access-related callback functions
VOID ACCESS_BeforeMemoryRead(MEMREADAVFUNPTR callback);
VOID ACCESS_BeforeMemoryRead(MEMREADAVLFUNPTR callback);
VOID ACCESS_BeforeMemoryRead(MEMREADAVOFUNPTR callback);
VOID ACCESS_BeforeMemoryRead(MEMREADAVIOFUNPTR callback);
VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVFUNPTR callback);
VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVLFUNPTR callback);
VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVOFUNPTR callback);
VOID ACCESS_BeforeMemoryWrite(MEMWRITEAVIOFUNPTR callback);
VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVFUNPTR callback);
VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVLFUNPTR callback);
VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVOFUNPTR callback);
VOID ACCESS_BeforeAtomicUpdate(MEMUPDATEAVIOFUNPTR callback);

VOID ACCESS_AfterMemoryRead(MEMREADAVFUNPTR callback);
VOID ACCESS_AfterMemoryRead(MEMREADAVLFUNPTR callback);
VOID ACCESS_AfterMemoryRead(MEMREADAVOFUNPTR callback);
VOID ACCESS_AfterMemoryRead(MEMREADAVIOFUNPTR callback);
VOID ACCESS_AfterMemoryWrite(MEMWRITEAVFUNPTR callback);
VOID ACCESS_AfterMemoryWrite(MEMWRITEAVLFUNPTR callback);
VOID ACCESS_AfterMemoryWrite(MEMWRITEAVOFUNPTR callback);
VOID ACCESS_AfterMemoryWrite(MEMWRITEAVIOFUNPTR callback);
VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVFUNPTR callback);
VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVLFUNPTR callback);
VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVOFUNPTR callback);
VOID ACCESS_AfterAtomicUpdate(MEMUPDATEAVIOFUNPTR callback);

// Types without existing implementation
// Functions for retrieving information about accesses
//VOID ACCESS_GetLocation(ADDRINT ins, LOCATION& location); // this relies on the pin tool too much to be easily replacable
// it would basically require storing location of any instruction that was ever sent to the analysers 

// Definitions of synchronisation-related callback functions
typedef VOID (*LOCKFUNPTR)(THREADID tid, LOCK lock);
//typedef VOID (*CONDFUNPTR)(THREADID tid, COND condition);
typedef VOID (*JOINFUNPTR)(THREADID tid, THREADID jtid);

// Functions for registering synchronisation-related callback functions
VOID SYNC_BeforeLockAcquire(LOCKFUNPTR callback);
VOID SYNC_BeforeLockRelease(LOCKFUNPTR callback);
/*API_FUNCTION VOID SYNC_BeforeSignal(CONDFUNPTR callback);
API_FUNCTION VOID SYNC_BeforeWait(CONDFUNPTR callback);*/
VOID SYNC_BeforeJoin(JOINFUNPTR callback);

VOID SYNC_AfterLockAcquire(LOCKFUNPTR callback);
VOID SYNC_AfterLockRelease(LOCKFUNPTR callback);
/*API_FUNCTION VOID SYNC_AfterSignal(CONDFUNPTR callback);
API_FUNCTION VOID SYNC_AfterWait(CONDFUNPTR callback);*/
VOID SYNC_AfterJoin(JOINFUNPTR callback);

// Definitions of thread-related special data types
typedef std::vector<stack_frame> Backtrace;
typedef std::vector< std::string > Symbols;


// Definitions of thread-related callback functions
typedef VOID (*THREADFUNPTR)(THREADID tid);
typedef VOID (*FORKFUNPTR)(THREADID tid, THREADID ntid);
typedef VOID (*ARG1FUNPTR)(THREADID tid, ADDRINT* arg);


// Functions for registering thread-related callback functions
VOID THREAD_ThreadStarted(THREADFUNPTR callback);
//VOID THREAD_ThreadFinished(THREADFUNPTR callback);
VOID THREAD_ThreadForked(FORKFUNPTR callback);
/*
API_FUNCTION VOID THREAD_FunctionEntered(THREADFUNPTR callback);
API_FUNCTION VOID THREAD_FunctionExited(THREADFUNPTR callback);

API_FUNCTION VOID THREAD_FunctionExecuted(const char* name, ARG1FUNPTR beforecb,
  UINT32 arg, ARG1FUNPTR aftercb);
API_FUNCTION VOID THREAD_FunctionExecuted(const char* name, ARG1FUNPTR callback,
  UINT32 arg);
*/
// Functions for retrieving information about threads
VOID THREAD_GetBacktrace(THREADID tid, Backtrace& bt);
VOID THREAD_GetBacktraceSymbols(Backtrace& bt, Symbols& symbols);
VOID THREAD_GetThreadCreationLocation(THREADID tid,
  std::string& location);
/*VOID THREAD_GetCurrentFunction(THREADID tid,
  std::string& function);
THREADID THREAD_GetThreadId();
std::thread::id THREAD_GetThreadUid();*/
/*
// Definitions of TM-related callback functions
typedef VOID (*BEFORETXSTARTFUNPTR)(THREADID tid);
typedef VOID (*BEFORETXCOMMITFUNPTR)(THREADID tid);
typedef VOID (*BEFORETXABORTFUNPTR)(THREADID tid);
typedef VOID (*BEFORETXREADFUNPTR)(THREADID tid, ADDRINT addr);
typedef VOID (*BEFORETXWRITEFUNPTR)(THREADID tid, ADDRINT addr);

typedef VOID (*AFTERTXSTARTFUNPTR)(THREADID tid, ADDRINT* result);
typedef VOID (*AFTERTXCOMMITFUNPTR)(THREADID tid, ADDRINT* result);
typedef VOID (*AFTERTXABORTFUNPTR)(THREADID tid, ADDRINT* result);
typedef VOID (*AFTERTXREADFUNPTR)(THREADID tid, ADDRINT addr);
typedef VOID (*AFTERTXWRITEFUNPTR)(THREADID tid, ADDRINT addr);

// Functions for registering TM-related callback functions
API_FUNCTION VOID TM_BeforeTxStart(BEFORETXSTARTFUNPTR callback);
API_FUNCTION VOID TM_BeforeTxCommit(BEFORETXCOMMITFUNPTR callback);
API_FUNCTION VOID TM_BeforeTxAbort(BEFORETXABORTFUNPTR callback);
API_FUNCTION VOID TM_BeforeTxRead(BEFORETXREADFUNPTR callback);
API_FUNCTION VOID TM_BeforeTxWrite(BEFORETXWRITEFUNPTR callback);

API_FUNCTION VOID TM_AfterTxStart(AFTERTXSTARTFUNPTR callback);
API_FUNCTION VOID TM_AfterTxCommit(AFTERTXCOMMITFUNPTR callback);
API_FUNCTION VOID TM_AfterTxAbort(AFTERTXABORTFUNPTR callback);
API_FUNCTION VOID TM_AfterTxRead(AFTERTXREADFUNPTR callback);
API_FUNCTION VOID TM_AfterTxWrite(AFTERTXWRITEFUNPTR callback);
*/


/*definitions needed for anaconda, related to PIN api */
#define PIN_MUTEX std::mutex
#define PIN_MutexLock(mutex) (*mutex).lock()
#define PIN_MutexUnlock(mutex) (*mutex).unlock()
#define PIN_MutexInit(mutex) ((void)0)

#define CONSOLE_NOPREFIX(message) std::cout << message


#endif //__LLVMTOOL_LACHESIS_TOOLAPI__