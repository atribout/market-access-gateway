#pragma once

#include <cstdint>
#include <bit>
#include <span>
#include "Messages.h"

class NasdaqItchParser {
private:
    mutable uint64_t internalSeqNum{0};

    struct TickerEntry {
        uint64_t ticker{0};
        uint16_t id{999};
    };

    static constexpr size_t TICKER_CAPACITY = 2048;
    static constexpr size_t TICKER_MASK = TICKER_CAPACITY - 1;

    mutable std::array<TickerEntry, TICKER_CAPACITY> tickerMap{};
    mutable uint16_t nextInstrumentId{0};

    inline uint16_t mapTickerToInstrument(uint64_t tickerInt) const noexcept {
        size_t idx = (tickerInt ^ (tickerInt >> 27)) & TICKER_MASK;
        
        while (tickerMap[idx].ticker != 0) {
            if (tickerMap[idx].ticker == tickerInt) {
                return tickerMap[idx].id;
            }
            idx = (idx + 1) & TICKER_MASK;
        }
        
        if (nextInstrumentId >= 1000) [[unlikely]] return 999; 
        
        tickerMap[idx].ticker = tickerInt;
        tickerMap[idx].id = nextInstrumentId++;
        return tickerMap[idx].id;
    }

public:
    bool parse(std::span<const uint8_t> payload, QueueItem* slot) const noexcept {
        if (payload.empty()) [[unlikely]] return false;

        const auto len = payload.size();
        const uint8_t* ptr = payload.data();

        const char messageType = static_cast<char>(ptr[0]);

        switch (messageType) {
            case 'A':
            {
                if (len < 36) [[unlikely]] return false;

                uint64_t rawTicker = *reinterpret_cast<const uint64_t*>(ptr + 24);
                uint16_t instrId = mapTickerToInstrument(rawTicker);

                if (instrId == 999) return false;
                
                slot->seqNum = ++internalSeqNum;
                slot->type = MsgType::AddOrder;
                slot->id = std::byteswap(*reinterpret_cast<const uint64_t*>(ptr + 11));                
                slot->side = (ptr[19] == 'B') ? Side::Buy : Side::Sell;                
                slot->quantity = std::byteswap(*reinterpret_cast<const uint32_t*>(ptr + 20));                
                slot->instrumentId = instrId;               
                slot->price = static_cast<int32_t>(std::byteswap(*reinterpret_cast<const uint32_t*>(ptr + 32)));
                
                return true;
            }

            case 'X': 
            {
                if (len < 40) [[unlikely]] return false;
                
                slot->seqNum = ++internalSeqNum;
                slot->type = MsgType::ExecutedOrder;                
                slot->id = std::byteswap(*reinterpret_cast<const uint64_t*>(ptr + 11));               
                slot->quantity = std::byteswap(*reinterpret_cast<const uint32_t*>(ptr + 19));
                
                return true;
            }

            case 'D':
            {
                if (len < 19) [[unlikely]] return false;
                
                slot->seqNum = ++internalSeqNum;
                slot->type = MsgType::CancelOrder;
                slot->id = std::byteswap(*reinterpret_cast<const uint64_t*>(ptr + 11));
                
                return true;
            }

            case 'U':
            {
                return false; 
            }

            default:
                return false;
        }
    }
};