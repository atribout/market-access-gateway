#pragma once
#include <cstdint>
#include <iostream>
#include "Messages.h"

struct Order {
    uint64_t id;
    int32_t price;
    uint32_t quantity;

    int32_t prev = -1;
    int32_t next = -1;

    uint16_t instrumentId;
    Side side;

    Order() = default;

    Order(uint64_t id, int32_t price, uint32_t quantity, Side side, uint16_t instId)
        : id(id), price(price), quantity(quantity), prev(-1), next (-1), instrumentId(instId), side(side) {}

    friend std::ostream& operator<<(std::ostream& os, const Order& order) {
        return os << "Instrument " << order.instrumentId << " : "<< "Order[" << order.id << "] " 
                  << (order.side == Side::Buy ? "BUY" : "SELL") << " "
                  << order.quantity << " @ " << order.price;
    }
};

enum class RejectReason : uint8_t {
    DuplicateId,
    InvalidPrice,
    InvalidQuantity,
    OrderNotFound,
    SystemFull
};