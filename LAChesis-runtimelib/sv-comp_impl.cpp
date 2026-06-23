#include "tool.h"
#include "utils.h"
#include <mutex>


static std::mutex global_lock;

/* SV-COMP VERIFIER function implementations*/
extern "C"{
    void __VERIFIER_atomic_begin(){
        global_lock.lock();
    }
    
    void  __VERIFIER_atomic_end(){
        global_lock.unlock();
    }


    void __VERIFIER_atomic_aquire(){
        global_lock.lock();
    }
    
    void  __VERIFIER_atomic_release(){
        global_lock.unlock();
    }
    
    unsigned long __VERIFIER_nondet_ulong(){
        int type = random_number(0,3);
        switch (type)
        {
        case 0:
            return 0;
            break;
        case 1:
            return 1;
            break;
        case 2:
            return ULONG_MAX/4;
            break;
        }
        return random_number(0ul,ULONG_MAX);
    }

    long __VERIFIER_nondet_long(){
        return __VERIFIER_nondet_ulong();
    }

    int __VERIFIER_nondet_int(){
        return __VERIFIER_nondet_ulong();
    }

    unsigned char __VERIFIER_nondet_uchar(){
        return __VERIFIER_nondet_ulong();
    }

    void __VERIFIER_nondet_memory(void *mem, size_t size) {
        unsigned char *p = (unsigned char *)mem;
        for (size_t i = 0; i < size; i++) {
            p[i] = __VERIFIER_nondet_uchar();
    }
}

    void __VERIFIER_assert_lachesis(int expression){
        if (!expression)
        {
            CONSOLE_NOPREFIX("\n Assert failed \n");        
        }
    }

}
