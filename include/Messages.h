#pragma once
#include <cstdint>

enum class MsgType : uint8_t 
{
    AddOrder = 'A',
    CancelOrder = 'C',
    ExecutedOrder = 'E'
};

struct alignas(32) QueueItem
{
    uint64_t seqNum;
    uint64_t id;
    int32_t price;
    uint32_t quantity;
    MsgType type;
    char side;
};