#pragma once
#include "Messages.h"
#include <cstdint>

namespace Sim {
    
    #pragma pack(push, 1)

    struct PacketHeader
    {
        uint64_t seqNum;
        MsgType type; // 'A' (Add), 'C' (Cancel), 'E' (Executed)
    };

    struct AddOrderMsg 
    {
        PacketHeader header;
        uint64_t id;
        int32_t price;
        uint32_t quantity;
        char side;
    };

    struct CancelOrderMsg 
    {
        PacketHeader header;
        uint64_t id;
    };

    #pragma pack(pop)

} // namespace Sim