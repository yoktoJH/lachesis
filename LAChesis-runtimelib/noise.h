#ifndef __LLVMTOOL_LACHESIS_NOISE__
#define __LLVMTOOL_LACHESIS_NOISE__
#include "callback.h"
#include "lachesis_types.h"
#include "config.h"

#include <cstdint>
#include <random>
#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>
#include <cstring>


class inverse_noise_lock
{

  std::atomic<std::chrono::time_point<std::chrono::steady_clock>> _end = std::chrono::steady_clock::now();

public:
  void wait()
  {
    std::this_thread::sleep_until(_end.load());
  }

  std::chrono::time_point<std::chrono::steady_clock> block_other_threads(std::chrono::milliseconds block_duration)
  {
    auto e = _end.load();
    auto end = std::chrono::steady_clock::now() + block_duration;
    while (!_end.compare_exchange_weak(e, end))
    {
      std::this_thread::sleep_until(e);
      end = std::chrono::steady_clock::now() + block_duration;
    }
    return end;
  }
};

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

// generator of inverse noise
static inverse_noise_lock inverse_lock;


struct noise_generator
{
  static void insert_noise(noise_type noise_t, std::uint32_t frequency, std::uint32_t strength_templ, bool random_strength)
  {
    // wait for the end of inverse noise
    inverse_lock.wait();

    if (noise_t == noise_type::NONE)
    {
      return;
    }

    if (random_number(0, 999) > frequency)
    {
      return;
    }
    // std::cout<<"noise is generated"<<std::endl;
    std::uint32_t strength = strength_templ;

    if (random_strength)
    {
      strength = random_number<std::uint32_t>(0, strength_templ);
    }

    if (noise_t == noise_type::SLEEP)
    {
      // std::cout<<"thread is sleeping"<<std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(strength));
    }

    if (noise_t == noise_type::YIELD)
    {
      while (strength-- > 0)
      {
        std::this_thread::yield();
      }
    }

    if (noise_t == noise_type::WAIT)
    {
      auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(strength);
      while (std::chrono::steady_clock::now() < end)
      {
        strength++;
      }
    }

    if (noise_t == noise_type::INVERSE)
    {
      inverse_noise(frequency, strength_templ, random_strength);
    }

    if (noise_t == noise_type::DEBUG)
    {
      std::cout << "frequency= " << frequency << " | strength= " << strength << " | random= " << random_strength << "\n";
    }
  }

  static void inverse_noise(std::uint32_t frequency, std::uint32_t strength, bool random_strength)
  {

    thread_local bool active_noise = false;
    thread_local std::chrono::time_point<std::chrono::steady_clock> end;

    if (active_noise)
    {
      if (end < std::chrono::steady_clock::now())
      {
        active_noise = false;
      }

      return;
    }

    if (random_number(0, 999) > frequency)
    {
      // no noise to be generated, wait like others
      inverse_lock.wait();
      return;
    }

    if (random_strength)
    {
      strength = random_number<std::uint32_t>(0, strength);
    }
    // generate inverse noise
    end = inverse_lock.block_other_threads(std::chrono::milliseconds(strength));
    active_noise = true;
  }
};


template <memory_operation_t op>
void insert_noise(char * name, int32_t line)
{
  
  std::vector<memop_filter>& filters = get_filters(op);
  noise_config_t noise = get_default_noise(op);
  for (memop_filter& filter: filters){
    if (filter.line == line && std::strcmp(filter.variable_name.c_str(),name)==0){
      noise = get_targeted_noise(op);
      break;
    }
  }

  noise_generator::insert_noise(noise.type,noise.frequency,noise.strength,noise.random);

}

struct noise_filter{
  memory_operation_t op;
  char* name;
  int32_t line;
};



#endif //__LLVMTOOL_LACHESIS_NOISE__