#pragma once
#include <atomic>
#include "RingBuffer.h"
#include "Messages.h"

extern RingBuffer<QueueItem, 1048576> ringBuffer;
extern std::atomic<bool> running;