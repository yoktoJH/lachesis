#include "tool.h"
#include "utils.h"
#include <mutex>
#include <ryml_all.hpp>
#include "uuid/uuid.h"
#include <assert.h>

static std::mutex global_lock;

/* SV-COMP VERIFIER function implementations*/
extern "C"{
    void __VERIFIER_atomic_begin(){
        global_lock.lock();
    }
    
    void  __VERIFIER_atomic_end(){
        global_lock.unlock();
    }
    
    unsigned long __VERIFIER_nondet_ulong(){
        int type = random_number(0,4);
        switch (type)
        {
        case 0:
            return 0;
	case 1:
	    return 1;
        case 2:
            return 2;
        case 3:
            return random_number(3ul,128ul);
        }
        return random_number(129ul,ULONG_MAX);
    }

    long __VERIFIER_nondet_long(){
        return __VERIFIER_nondet_ulong();
    }

    long long __VERIFIER_nondet_longlong(){
        return __VERIFIER_nondet_ulong();
    }

    unsigned int __VERIFIER_nondet_uint(){
        return __VERIFIER_nondet_ulong();
    }

    int __VERIFIER_nondet_int(){
        return __VERIFIER_nondet_ulong();
    }

    unsigned char __VERIFIER_nondet_uchar(){
        return __VERIFIER_nondet_ulong();
    }

    char __VERIFIER_nondet_char(){
	return __VERIFIER_nondet_ulong();
    }

    unsigned char __VERIFIER_nondet_bool(){
        return (bool) random_number(0,1);
    }

    void __VERIFIER_nondet_memory(void *mem, size_t size){
        unsigned char *p = (unsigned char *)mem;
        for (size_t i = 0; i < size; i++) 
        {
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


