#include <cstdlib>
#include <emmintrin.h>
#include <thread>
#include <atomic>
#include <cstring>
#include <print>
#include <immintrin.h>

#include "OrderBook.h"
#include "Order.h"
#include "Listeners.h"
#include "include/Messages.h"
#include "include/NetworkProducer.h"
#include "include/RingBuffer.h"
#include "include/TSCClock.h"
#include "include/Receivers.h"
#include "include/SimParser.h"

constexpr size_t BUFFER_SIZE = 4096;
RingBuffer<QueueItem, BUFFER_SIZE> ringBuffer;
std::atomic<bool> running{true};

void consumer_thread()
{
    pin_to_core(5);
    TSCClock::get().printCalibration();
    std::println("Engine started (waiting for data)...");

    EmptyListener listener;
    OrderBook<EmptyListener> lob(listener);

    // We store the latencies to print the percentiles later
    std::vector<uint64_t> samples;
    samples.reserve(100000);

    uint64_t maxQueueDepth = 0;

    // For packet loss
    uint64_t lastSeqNum = 0;
    uint64_t gapCount = 0;

    unsigned int dummy;
    uint64_t start_cycles, end_cycles;

    while (running) 
    {
        size_t currentDepth = ringBuffer.getSize();
        if (currentDepth > maxQueueDepth) maxQueueDepth = currentDepth;

        QueueItem* item = ringBuffer.peek();
        if (item) 
        {
            if(item->seqNum <= lastSeqNum)
            {
                lastSeqNum = item->seqNum;
            }
            else if(lastSeqNum != 0 && item->seqNum != lastSeqNum + 1)
            {
                gapCount += (item->seqNum -lastSeqNum -1);
            }
            lastSeqNum = item->seqNum;

            start_cycles = __rdtscp(&dummy);
            // --- CRITICAL ZONE ---
            if(item->type == MsgType::AddOrder) {
                Side side = (item->side == 'B') ? Side::Buy : Side::Sell;
                Order newOrder(item->id, item->price, item->quantity, side);
                lob.submitOrder(newOrder);
            }
            else if (item->type == MsgType::CancelOrder) {
                lob.cancelOrder(item->id);
            }
            // -----------------------------------------

            end_cycles = __rdtscp(&dummy);

            ringBuffer.advance();

            uint64_t cycles = end_cycles - start_cycles;
            samples.push_back(cycles);

            if (samples.size() > 100000) {

                std::sort(samples.begin(), samples.end());

                double p50 = TSCClock::get().toNanos(samples[50000]);
                double p99 = TSCClock::get().toNanos(samples[99000]);
                double maxLat = TSCClock::get().toNanos(samples.back());

                std::println("--- STATS REPORT ---");
                std::println("Lat p50   : {} ns", p50);
                std::println("Lat p99   : {} ns", p99);
                std::println("Lat Max   : {} ns", maxLat);
                std::println("Queue Max : {} / {}", maxQueueDepth, BUFFER_SIZE);
                std::println("Packet Loss : {}", gapCount);

                samples.clear();
                maxQueueDepth = 0;
            }
        } 
        else 
        {
            _mm_pause();
        }
    }
}

int main(int argc, char* argv[])  {
    std::string mode = "live";
    if (argc > 1) {
        mode = argv[1];
    }

    std::thread consumer(consumer_thread);

    if (mode == "pcap") {
        std::println("=== Starting in REPLAY mode (PCAP) ===");
        PcapReceiver pcapRecv("nasdaq_sample.pcap");
        // NetworkProducer<PcapReceiver> producer(pcapRecv);
        // producer.run();
    }
    else {
        std::println("=== Starting in LIVE mode (UDP Multicast) ===");
        NetworkProducer<SimParser, UdpMulticastReceiver> producer(SimParser{}, 1234);
        producer.run();
    }
        
    consumer.join();
    return 0;
}
