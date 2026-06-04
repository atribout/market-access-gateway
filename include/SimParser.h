#pragma once
#include <cstddef>
#include "SimProtocol.h"
#include "Messages.h"
#include "Globals.h"

class SimParser {
public:
    inline bool parse(const char* packet_ptr, size_t len, QueueItem* slot) {
        if (len < sizeof(Sim::PacketHeader)) return false;

        const Sim::PacketHeader* header = reinterpret_cast<const Sim::PacketHeader*>(packet_ptr);

        if (header->type == MsgType::AddOrder && len >= sizeof(Sim::AddOrderMsg)) {
            const Sim::AddOrderMsg* msg = reinterpret_cast<const Sim::AddOrderMsg*>(packet_ptr);
            slot->type = MsgType::AddOrder;
            slot->seqNum = header->seqNum;
            slot->id = msg->id;
            slot->price = msg->price;
            slot->quantity = msg->quantity;
            slot->side = msg->side;
            ringBuffer.publish();
            return true;
        }
        else if (header->type == MsgType::CancelOrder && len >= sizeof(Sim::CancelOrderMsg))  {
            const Sim::CancelOrderMsg* msg = reinterpret_cast<const Sim::CancelOrderMsg*>(packet_ptr);
            slot->type = MsgType::CancelOrder;
            slot->seqNum = header->seqNum;
            slot->id = msg->id;
            ringBuffer.publish();
            return true;
        }
        else if (header->type == MsgType::ExecutedOrder && len >= sizeof(Sim::ExecutedOrderMsg))  {
            const Sim::ExecutedOrderMsg* msg = reinterpret_cast<const Sim::ExecutedOrderMsg*>(packet_ptr);
            slot->type = MsgType::ExecutedOrder;
            slot->seqNum = header->seqNum;
            slot->id = msg->id;
            slot->quantity = msg->quantity;
            ringBuffer.publish();
            return true;
        }

        return false; //Unknown type or corrupted packet
    }
};