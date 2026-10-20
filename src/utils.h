

#ifndef __LLVMTOOL_LACHESIS_UTILS__
#define __LLVMTOOL_LACHESIS_UTILS__

#include <concepts>
#include <random>

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
random_number(T min, T max)
{

  static std::random_device rd;
  static std::ranlux24_base gen(rd());

  static std::mutex random_lock;
  std::uniform_int_distribution<> distrib(min, max);

  const std::lock_guard guard(random_lock);

  return distrib(gen);
}

#endif //__LLVMTOOL_LACHESIS_UTILS__
