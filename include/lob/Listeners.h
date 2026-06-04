#pragma once
#include <cstdint>
#include <vector>
#include <print>
#include "Order.h"

struct ConsoleListener 
{
    void onOrderAdded(uint16_t instrId, uint64_t id, int32_t price, uint32_t qty, Side side) 
    {
        std::println("[INSTR {}] [ORDER] New {} {} @ {} (ID: {})", 
                     instrId, (side == Side::Buy ? "Buy" : "Sell"), qty, price, id);
    }

    void onOrderCancelled(uint16_t instrId, uint64_t id) 
    {
        std::println("[INSTR {}] [CANCEL] Order {} removed", instrId, id);
    }

    void onOrderExecuted(uint16_t instrId, uint64_t id, uint32_t qty) {
        std::println("[INSTR {}] [EXEC] Order {} passively executed for {} lots", instrId, id, qty);
    }

    void onOrderRejected(uint16_t instrId, uint64_t id, RejectReason reason)
    {
        std::println("[INSTR {}] [REJ] Order {} rejected (Reason: {})", instrId, id, static_cast<int>(reason));
    }

    void onTrade(uint16_t instrId, uint64_t aggId, uint64_t passId, int32_t price, uint32_t qty) 
    {
        std::println("[INSTR {}] >>> TRADE EXECUTE: {}@{} (Aggressor: {}, Passive: {})", 
                     instrId, qty, price, aggId, passId);
    }

    // --- PUBLIC FLOW ---
    void onOrderBookUpdate(uint16_t instrId, int32_t price, uint32_t volume, Side side)
    {
        std::println("[INSTR {}] [MKT DATA] Price Level {} ({}) is now {}", 
                     instrId, price, (side == Side::Buy ? "Bid" : "Ask"), volume);
    }
};

struct EmptyListener
{
    void onOrderAdded(uint16_t, uint64_t, int32_t, uint32_t, Side) {}

    void onOrderCancelled(uint16_t, uint64_t) {}

    void onOrderExecuted(uint16_t, uint64_t, uint32_t) {}

    void onOrderRejected(uint16_t, uint64_t, RejectReason) {}

    void onTrade(uint16_t, uint64_t, uint64_t, int32_t, uint32_t) {}

    void onOrderBookUpdate(uint16_t, int32_t, uint32_t, Side) {}
};

struct VectorListener {
    struct TradeInfo 
    { 
        uint16_t instrId;
        uint64_t aggId;
        uint64_t passId;
        int32_t price;
        uint32_t qty; 
    };

    struct OrderRef 
    {
        uint16_t instrId;
        uint64_t id;
    };

    struct RejectInfo 
    {
        uint16_t instrId;
        uint64_t id;
        RejectReason reason;
    };

    std::vector<TradeInfo> trades;
    std::vector<OrderRef> cancelledOrders;
    std::vector<RejectInfo> rejectedOrders;

    void onTrade(uint16_t instrId, uint64_t aggId, uint64_t passId, int32_t price, uint32_t qty) 
    {
        trades.push_back({instrId, aggId, passId, price, qty});
    }

    void onOrderCancelled(uint16_t instrId, uint64_t id) 
    {
        cancelledOrders.push_back({instrId, id});
    }

    void onOrderExecuted(uint16_t instrId, uint64_t id, uint32_t qty) {}

    void onOrderRejected(uint16_t instrId, uint64_t id, RejectReason reason) 
    {
        rejectedOrders.push_back({instrId, id, reason});
    }
    
    void onOrderAdded(uint16_t, uint64_t, int32_t, uint32_t, Side) {}
    void onOrderBookUpdate(uint16_t, int32_t, uint32_t, Side) {}

    void clear() 
    {
        trades.clear();
        cancelledOrders.clear();
        rejectedOrders.clear();
    }
};