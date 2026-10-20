#ifndef __LLVMTOOL_LACHESIS_LTYPES__
#define __LLVMTOOL_LACHESIS_LTYPES__
#include <string>
#include <cstdint>

enum class noise_type
{
  NONE,
  SLEEP,
  YIELD,
  WAIT,
  INVERSE,
  DEBUG
};

struct noise_config_t
{
    noise_type type;
    int frequency;
    int strength;
    bool random;
};

struct memop_filter
{
    std::string file;
    std::string variable_name;
    int32_t line;
};


enum class memory_operation_t
{
    READ,
    WRITE,
    UPDATE,
};

enum class memory_callback_t
{
    A,
    AV,
    AVL,
    AVO,
    AVIO,
};

enum class lock_operation_t {
    LOCK,
    UNLOCK
};

enum class HookPosition { Before, After };
#endif //__LLVMTOOL_LACHESIS_LTYPES__