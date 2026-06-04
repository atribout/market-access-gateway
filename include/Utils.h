#pragma once
#include <cstdint>
#include <print>
#include <x86intrin.h>
#include <sched.h>
#include <pthread.h>

inline uint64_t rdtsc()
{
    unsigned int dummy;
    return __rdtscp(&dummy);
}

inline void pin_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0) {
        std::println(stderr, "Error pinning thread to core {}", core_id);
    } else {
        std::println("Thread pinned to Core {}", core_id);
    }
}