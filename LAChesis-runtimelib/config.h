#ifndef __LLVMTOOL_LACHESIS_CONFIG__
#define __LLVMTOOL_LACHESIS_CONFIG__
#include "lachesis_types.h"

#include <cstdint>
#include <vector>

std::vector<memop_filter>& get_filters(memory_operation_t op);

noise_config_t& get_default_noise(memory_operation_t op);

noise_config_t& get_targeted_noise(memory_operation_t op);
void load_config();
#endif //__LLVMTOOL_LACHESIS_CONFIG__
