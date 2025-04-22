#ifndef __LLVMTOOL_LACHESIS_NOISE__
#define __LLVMTOOL_LACHESIS_NOISE__
#include "callback.h"

#include <cstdint>
#include <random>
#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>

#define WRITE_TYPE NoiseType::NONE
#define WRITE_FREQUENCY 500
#define WRITE_STRENGTH 20
#define WRITE_STRENGTH_RANDOM false

#define READ_TYPE NoiseType::NONE
#define READ_FREQUENCY 320
#define READ_STRENGTH 200
#define READ_STRENGTH_RANDOM false

#define UPDATE_TYPE NoiseType::DEBUG
#define UPDATE_FREQUENCY 0
#define UPDATE_STRENGTH 0
#define UPDATE_STRENGTH_RANDOM false

enum class NoiseType
{
  NONE,
  SLEEP,
  YIELD,
  WAIT,
  INVERSE,
  DEBUG
};

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
    while (_end.compare_exchange_weak(e, end))
    {
      std::this_thread::sleep_until(e);
      end = std::chrono::steady_clock::now() + block_duration;
    }
    return end;
  }
};

template <typename IntType>
IntType random_number(IntType min, IntType max)
{

  static std::random_device rd;
  static std::ranlux24_base gen(rd());

  static std::mutex random_lock;
  std::uniform_int_distribution<> distrib(min, max);

  const std::lock_guard guard(random_lock);

  return distrib(gen);
}

// generator of inverse noise
static inverse_noise_lock inverse_noise;

// maybe make this a function class, as it would make more sense
template <std::uint32_t frequency, std::uint32_t strength_templ, bool random_strength>
struct inverse_noise_generator
{
  void insert_noise()
  {

    static thread_local bool active_noise = false;
    static thread_local std::chrono::time_point<std::chrono::steady_clock> end;

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
      // no noise is to be generated, wait like others
      inverse_noise.wait();
      return;
    }

    std::uint32_t strength = strength_templ;

    if constexpr (random_strength)
    {
      strength = random_number<std::uint32_t>(0, strength_templ);
    }
    // generate inverse noise
    end = inverse_noise.block_other_threads(std::chrono::milliseconds(strength));
    active_noise = true;
  }
};

template <NoiseType noise_t, std::uint32_t frequency, std::uint32_t strength_templ, bool random_strength>
struct noise_generator
{
  void insert_noise()
  {
    // wait for the end of inverse noise
    inverse_noise.wait();
    if constexpr (noise_t == NoiseType::NONE)
    {
      return;
    }
    if (random_number(0, 999) > frequency)
    {
      return;
    }
    // std::cout<<"noise is generated"<<std::endl;
    std::uint32_t strength = strength_templ;

    if constexpr (random_strength)
    {
      strength = random_number<std::uint32_t>(0, strength_templ);
    }

    if constexpr (noise_t == NoiseType::SLEEP)
    {
      // std::cout<<"thread is sleeping"<<std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(strength));
    }

    if constexpr (noise_t == NoiseType::YIELD)
    {
      while (strength-- > 0)
      {
        std::this_thread::yield();
      }
    }

    if constexpr (noise_t == NoiseType::WAIT)
    {
      auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(strength);
      while (std::chrono::steady_clock::now() < end)
      {
        strength++;
      }
    }
    if constexpr (noise_t == NoiseType::INVERSE)
    {
      static inverse_noise_generator<frequency, strength_templ, random_strength> gen;
      gen.insert_noise_impl();
    }

    if constexpr(noise_t == NoiseType::DEBUG){
      std::cout<< "frequency= " << frequency <<" | strength= "<< strength << " | random= "<< random_strength<<"\n";
    }
  }
};

template <memory_operation_type op>
void insert_noise()
{

  if constexpr (op == memory_operation_type::READ)
  {
    static noise_generator<READ_TYPE, READ_FREQUENCY, READ_STRENGTH, READ_STRENGTH_RANDOM> read_generator;
    read_generator.insert_noise();
    return;
  }

  if constexpr (op == memory_operation_type::WRITE)
  {
    static noise_generator<WRITE_TYPE, WRITE_FREQUENCY, WRITE_STRENGTH, WRITE_STRENGTH_RANDOM> write_geneator;
    write_geneator.insert_noise();
    return;
  }

  if constexpr (op == memory_operation_type::UPDATE)
  {
    static noise_generator<UPDATE_TYPE, UPDATE_FREQUENCY, UPDATE_STRENGTH, UPDATE_STRENGTH_RANDOM> update_generator;
    update_generator.insert_noise();
  }
}

#endif //__LLVMTOOL_LACHESIS_NOISE__