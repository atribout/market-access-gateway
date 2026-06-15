#pragma once
#include <bit>
#include <span>
#include "SimProtocol.h"
#include "Messages.h"

class SimParser {
public:
    bool parse(std::span<const uint8_t> payload, QueueItem* slot) {
        auto len = payload.size();
        if (len < sizeof(Sim::PacketHeader)) [[unlikely]] return false;

        const uint8_t* packet_ptr = payload.data();
        const Sim::PacketHeader* header = reinterpret_cast<const Sim::PacketHeader*>(packet_ptr);

        if (header->type == MsgType::AddOrder && len >= sizeof(Sim::AddOrderMsg)) {
            const Sim::AddOrderMsg* msg = reinterpret_cast<const Sim::AddOrderMsg*>(packet_ptr);
            slot->seqNum = std::byteswap(header->seqNum);
            slot->id = std::byteswap(msg->id);
            slot->price = std::byteswap(msg->price);
            slot->quantity = std::byteswap(msg->quantity);
            slot->instrumentId = std::byteswap(header->instrumentId);
            slot->type = MsgType::AddOrder;
            slot->side = (msg->side == 'B') ? Side::Buy : Side::Sell;
            return true;
        }
        else if (header->type == MsgType::CancelOrder && len >= sizeof(Sim::CancelOrderMsg))  {
            const Sim::CancelOrderMsg* msg = reinterpret_cast<const Sim::CancelOrderMsg*>(packet_ptr);
            slot->seqNum = std::byteswap(header->seqNum);
            slot->id = std::byteswap(msg->id);
            slot->instrumentId = std::byteswap(header->instrumentId);
            slot->type = MsgType::CancelOrder;
            return true;
        }
        else if (header->type == MsgType::ExecutedOrder && len >= sizeof(Sim::ExecutedOrderMsg))  {
            const Sim::ExecutedOrderMsg* msg = reinterpret_cast<const Sim::ExecutedOrderMsg*>(packet_ptr);
            slot->seqNum = std::byteswap(header->seqNum);
            slot->id = std::byteswap(msg->id);
            slot->quantity = std::byteswap(msg->quantity);
            slot->instrumentId = std::byteswap(header->instrumentId);
            slot->type = MsgType::ExecutedOrder;
            return true;
        }

        return false; //Unknown type or corrupted packet
    }
};