#pragma once
#include <print>
#include <immintrin.h>
#include "NetworkConcepts.h"
#include "Utils.h"
#include "RingBuffer.h"
#include "Globals.h"
#include "net/BinaryItchReceiver.h"

template<MessageParserConcept ParserT, PacketReceiverConcept ReceiverT>
class NetworkProducer {
private:
    ParserT parser;
    ReceiverT receiver;
public:

    template<typename... Args>
    explicit NetworkProducer(ParserT parser_inst, Args&&... receiver_args) 
        : parser(parser_inst),
          receiver(std::forward<Args>(receiver_args)...) {}

    void run() {
        pin_to_core(4);
        std::println("Network thread listening...");

        while (running) {
            auto payload = receiver.receive();

            if (payload.empty()) {
                if constexpr (std::is_same_v<ReceiverT, BinaryItchReceiver>) {
                    std::println("End of ITCH file reached.");
                    break;
                }
                else {
                    _mm_pause();
                    continue;
                }
            }

            QueueItem* slot = nullptr;
            while (!(slot = ringBuffer.claim())) {
                _mm_pause();
            };

            if (parser.parse(payload, slot))  {
                ringBuffer.publish();
            }
        }   
    }
};