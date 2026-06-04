#pragma once
#include <array>
#include <vector>
#include "PassiveOrderBook.h"
#include "OrderPool.h"

template<typename T>
concept TradeListenerConcept = requires(T t, uint16_t inst, uint64_t id, int32_t p, uint32_t q, Side s, RejectReason r) {
    { t.onTrade(inst, id, id, p, q) };
    { t.onOrderAdded(inst, id, p, q, s) };
    { t.onOrderCancelled(inst, id) };
    { t.onOrderExecuted(inst, id, q) };
    { t.onOrderRejected(inst, id, r) };
    { t.onOrderBookUpdate(inst, p, q, s) };
};

template<TradeListenerConcept ListenerT>
class MarketManager {
private:
    static constexpr size_t MAX_INSTRUMENTS = 64;
    static constexpr size_t MAX_LIVE_ORDERS = 1'000'000; 
    static constexpr size_t MAX_ORDER_IDS = 10'000'000;

    OrderPool pool;
    std::vector<int32_t> orderIndexLookup;

    std::array<PassiveOrderBook, MAX_INSTRUMENTS> books;

    ListenerT& listener;

public:
    MarketManager(ListenerT& l): listener(l), pool(MAX_LIVE_ORDERS), orderIndexLookup(MAX_ORDER_IDS, -1) {};
    
    inline void onAddOrder(uint16_t instrId, uint64_t id, int32_t price, uint32_t quantity, Side side) {
        if (id >= orderIndexLookup.size()) [[unlikely]] return;

        int32_t idx = pool.allocate(id, price, quantity, side, instrId);

        if (idx == -1) [[unlikely]] return;
        
        orderIndexLookup[id] = idx;

        uint32_t newVolume = books[instrId].addOrder(idx, pool);

        listener.onOrderAdded(instrId, id, price, quantity, side);
        listener.onOrderBookUpdate(instrId, price, newVolume, side);
    }

    inline void onCancelOrder(uint64_t id) {
        if (id >= orderIndexLookup.size()) [[unlikely]] return;

        int32_t idx = orderIndexLookup[id];

        if (idx == -1) [[unlikely]] return;

        Order& order = pool.get(idx);
        uint16_t instrId = order.instrumentId;

        uint32_t newVolume = books[instrId].reduceVolume(order, order.quantity);
        books[instrId].removeOrder(idx, pool);

        orderIndexLookup[id] = -1;

        listener.onOrderCancelled(instrId, id);
        listener.onOrderBookUpdate(instrId, order.price, newVolume, order.side);
        
        pool.deallocate(idx);
    }

    inline void onOrderExecuted(uint64_t id, uint32_t executedQty) {
        if (id >= orderIndexLookup.size()) [[unlikely]] return;

        int32_t idx = orderIndexLookup[id];

        if (idx == -1) [[unlikely]] return;

        Order& order = pool.get(idx);
        uint16_t instrId = order.instrumentId;

        uint32_t actualExecuted = std::min(order.quantity, executedQty);
        order.quantity -= actualExecuted;

        int32_t newVolume = books[instrId].reduceVolume(order, actualExecuted);

        listener.onOrderExecuted(instrId, id, actualExecuted);
        listener.onOrderBookUpdate(instrId, order.price, newVolume, order.side);

        if (order.quantity == 0) {
            books[instrId].removeOrder(idx, pool);
            orderIndexLookup[id] = -1;
            pool.deallocate(idx);
        }
    }
};
