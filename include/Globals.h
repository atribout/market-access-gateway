#pragma once
#include <atomic>
#include "RingBuffer.h"
#include "Messages.h"

extern RingBuffer<QueueItem, 4096> ringBuffer;
extern std::atomic<bool> running;