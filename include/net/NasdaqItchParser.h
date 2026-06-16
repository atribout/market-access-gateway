#pragma once

#include <cstdint>
#include <bit>
#include <span>
#include "Messages.h"

class NasdaqItchParser {
private:
    mutable uint64_t internalSeqNum{0};

    constexpr uint16_t mapTickerToInstrument(uint64_t tickerInt) const noexcept {
        switch (tickerInt) {
            case 2314885530818465365ULL: return 0; // UN
            case 2314885530821544274ULL: return 1; // RIO
            case 2314885530821607763ULL: return 2; // SAP
            case 2314885530821809742ULL: return 3; // NVS
            case 2314885530821543236ULL: return 4; // DEO
            default: return 999; // Ignore
        }
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