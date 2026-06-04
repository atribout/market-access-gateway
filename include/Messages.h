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
    int32_t price;     // Ignored if type == CancelOrder
    uint32_t quantity; // Ignored if type == CancelOrder
    MsgType type;
    char side;         // Ignored if type == CancelOrder
};