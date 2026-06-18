#pragma once
#include <vector>
#include "Order.h"
#include "OrderPool.h"
#include "BboBitset.h"
#include "PagedPriceArray.h"

class PassiveOrderBook {
private:

    struct Level {
        int32_t head = -1;
        int32_t tail = -1;
        uint32_t totalVolume = 0;
    };

    static constexpr size_t MAX_PRICE = 2000000;
    PagedPriceArray<Level, MAX_PRICE> bids;
    PagedPriceArray<Level, MAX_PRICE> asks;

    BboBitset bidPrices;
    BboBitset askPrices;

public:
    PassiveOrderBook() = default;

    uint32_t addOrder(int32_t idx, OrderPool& pool) {
        Order& order = pool.get(idx);
        auto& bookSide = (order.side == Side::Buy) ? bids: asks;
        Level& level = bookSide[order.price];
                
        if (level.head == -1) {
            level.head = idx;
            level.tail = idx;
        }
        else {
            pool.get(level.tail).next = idx;
            pool.get(idx).prev = level.tail;
            level.tail = idx;
        }

        level.totalVolume += order.quantity;

        if (level.totalVolume == order.quantity) {
            if (order.side == Side::Buy) bidPrices.setPrice(order.price);
            else askPrices.setPrice(order.price);
        }

        return level.totalVolume;
    }

    uint32_t removeOrder(uint32_t idx, OrderPool& pool) {
        Order& order = pool.get(idx);
        auto& bookSide = (order.side == Side::Buy) ? bids: asks;
        Level& level = bookSide[order.price];

        if (order.prev != -1) {
            pool.get(order.prev).next = order.next;
        }
        else {
            level.head = order.next;
        }
        
        if (order.next != -1) {
            pool.get(order.next).prev = order.prev;
        }
        else {
            level.tail = order.prev;
        }

        if (level.totalVolume == 0) {
            if (order.side == Side::Buy) bidPrices.clearPrice(order.price);
            else askPrices.clearPrice(order.price);
        }

        return level.totalVolume;
    }

    int getBestBid() const { return bidPrices.getBestBid(); }
    int getBestAsk() const { return askPrices.getBestAsk(); }

    uint32_t reduceVolume(Order& order, uint32_t qty) {
        auto& bookSide = (order.side == Side::Buy) ? bids: asks;
        bookSide[order.price].totalVolume -= qty;
        return bookSide[order.price].totalVolume;
    }
};