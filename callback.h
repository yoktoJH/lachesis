#ifndef __LLVMTOOL_LACHESIS_CALLBACK__
#define __LLVMTOOL_LACHESIS_CALLBACK__

#include "types.h"
#include <vector>

enum class memory_operation_type
{
    READ,
    WRITE,
    UPDATE,
};

enum class memory_callback_type
{
    A,
    AV,
    AVL,
    AVO,
    AVIO,
};

enum class lock_operation_type {
    LOCK,
    UNLOCK
};

struct memory_callback
{
    virtual void call(THREADID tid, ADDRINT addr, UINT32 size, const VARIABLE &variable, const LOCATION &location, BOOL is_local, ADDRINT ins) = 0;
    virtual ~memory_callback() = default;
};

template <typename T, memory_callback_type C>
struct memory_callback_impl : memory_callback
{
    T function;

    explicit memory_callback_impl(T fun) : function{fun} {}

    memory_callback_impl(const memory_callback_impl &other) : function{other.function} {}

    memory_callback_impl& operator=(const memory_callback_impl &other)
    {
        function = other.function;
        return *this;
    }

    memory_callback_impl(memory_callback_impl &&other) = delete;

    memory_callback_impl& operator=(memory_callback_impl &&other) = delete;

    void call(THREADID tid, ADDRINT addr, UINT32 size, const VARIABLE &variable, const LOCATION &location, BOOL is_local, ADDRINT ins) override
    {
        if constexpr (C == memory_callback_type::A)
        {
            function(tid, addr, size);
        }
        else if constexpr (C == memory_callback_type::AV)
        {
            function(tid, addr, size, variable);
        }
        else if constexpr (C == memory_callback_type::AVL)
        {
            function(tid, addr, size, variable, location);
        }
        else if constexpr (C == memory_callback_type::AVO)
        {
            function(tid, addr, size, variable, is_local);
        }
        else if constexpr (C == memory_callback_type::AVIO)
        {
            function(tid, addr, size, variable, ins, is_local);
        }
    }
    ~memory_callback_impl() override = default;
};

template <typename T>
struct callbacks_storage {
    std::vector<T> before = {};
    std::vector<T> after = {};

    callbacks_storage() = default;

    callbacks_storage(callbacks_storage &&other) : before(std::move(other.before)),
                                                                 after(std::move(other.after)) {
    }

    callbacks_storage &operator=(callbacks_storage &&other) noexcept {
        if (this != &other) {
            before = std::move(other.before);
            after = std::move(other.after);
        }
        return *this;
    }

    callbacks_storage(const callbacks_storage &) = delete;

    callbacks_storage &operator=(const callbacks_storage &) = delete;

    ~callbacks_storage() = default;
};

#endif //__LLVMTOOL_LACHESIS_CALLBACK__